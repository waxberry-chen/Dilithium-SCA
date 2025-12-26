from typing import Tuple
import numpy as np
from scipy import signal
from tqdm import tqdm as tq
import os
import json
from datetime import datetime
import matplotlib
import matplotlib.pyplot as plt

matplotlib.use('Agg')
plt.rcParams.update({
    'font.size': 15,                   # 默认字体大小
    'font.family': 'sans-serif',       # 字体系列
    'font.sans-serif': ['DejaVu Sans', 'Arial'],  # 无衬线字体优先级
    'font.serif': ['Times New Roman'], # 衬线字体
    'font.monospace': ['Courier New'], # 等宽字体
    'font.weight': 'normal',           # 字体粗细
    'axes.titlesize': 16,              # 坐标轴标题大小
    'axes.labelsize': 15,              # 坐标轴标签大小
    'xtick.labelsize': 13,             # x轴刻度标签大小
    'ytick.labelsize': 13,             # y轴刻度标签大小
    'legend.fontsize': 15,             # 图例字体大小
    'figure.titlesize': 18,            # 图形标题大小
    'figure.labelsize': 15             # 图形标签大小
})

# import json file
try:
    with open("setting.json") as setf:
        config = json.load(setf)
except:
    raise ValueError("Cant find setting.json file")

general = config.get("General",{})
path = config.get("PATH",{})
config_cpa = config.get("CPA",{})
config_process = config.get("PROCESS",{})
config_tvla = config.get("TVLA",{})
# import end

# vars
special_b = general.get("KEY_TRUE")
remark = general.get("REMARK")
SAMPLE_NUM = general.get("SAMPLE_NUM")
PICTURE_PATH = path.get("RESULT_ROOT")
DIR_TAG = general.get("DIR_TAG")
DATA_ROOT = path.get("DATA_ROOT")
TRACE_FILE_NAME = config_process.get("AVERAGED_FILE_PREFIX_NAME") + str(general.get("FILE_NUM")) + '.txt'
TRACE_PROCESS_MODE = general.get("TRACE_PROCESS_MODE")

def preprocess_traces(traces):
    """TVLA标准预处理流程"""
    # 1. 每条轨迹去均值（去除直流偏移）
    traces = traces - np.mean(traces, axis=1, keepdims=True)
    
    # 2. 可选：去趋势
    
    for i in range(traces.shape[0]):
        traces[i] = signal.detrend(traces[i])
    
    # 3. 可选：标准化每条轨迹
    traces = traces / np.std(traces, axis=1, keepdims=True)
    
    return traces

def down_sample(traces,down_num=5):
    return traces.reshape(traces.shape[0],down_num,traces.shape[1]//down_num).mean(axis=1)

def matrix_mean_var(trace_matrix):
    mean = np.mean(trace_matrix, axis=0, keepdims=True)
    var = np.var(trace_matrix, axis=0, keepdims=True, ddof=1) # div n-1
    return (mean,var)

class TVLA:
    def __init__(self,tvla_trace_file,
                 sample_number=5000,
                 traces_number = 5000,
                ) -> None:
        self.trace_file = tvla_trace_file
        if not os.path.isfile(tvla_trace_file):
            raise ValueError(f"Can not find file:{tvla_trace_file}")
        self.sample_number = sample_number
        self.traces_number = traces_number
        self.matrix_a = []
        self.matrix_b = []
        
        

    def read_traces(self):
        with tq(total=self.traces_number, desc=">>>>> 01 Reading Power traces") as read_bar:
            with open(self.trace_file,'r') as pf:
                number = 0
                # Read traces into matrix
                for line in pf:
                    if number >= self.traces_number or not line.strip():
                        break
                    else:
                        plaintext_str, power_trace_str = line.split(':', 1)
                        plaintext_number = int(plaintext_str)
                        power_trace = np.array(power_trace_str.strip().split()).astype(np.float64)
                        if plaintext_number%2:
                            self.matrix_a.append(power_trace)
                        else :
                            self.matrix_b.append(power_trace)   
                        number += 1
                        read_bar.update(1)
        print(f'Read {number} traces: fix (A):{len(self.matrix_a)} traces,random (B) {len(self.matrix_b)} traces')

    def get_tvla_result(self):
        if self.matrix_a is None or self.matrix_b is None:
            raise ValueError("Matrix A or Matrix B is without elements.")
        pre_a = preprocess_traces(self.matrix_a)
        pre_b = preprocess_traces(self.matrix_b)
        array_a = np.array(pre_a)
        array_b = np.array(pre_b)
        # array_a = down_sample(np.array(pre_a))
        # array_b = down_sample(np.array(pre_b))
        # print(array_a.shape)
        if array_a.shape[1] != array_b.shape[1]:
            raise ValueError("Sample number error: Matrix A != Matrix B")
        mva = matrix_mean_var(array_a)
        mvb = matrix_mean_var(array_b)
        na,nb = array_a.shape[0],array_b.shape[0]
        # print(na,nb)
        # print(mva,mvb)
        #t = (mva[0]-mvb[0])/np.sqrt(mva[1]/na + mvb[1]/nb)
        #t = (mva[0]-mvb[0])
        pooled_std = np.sqrt(((na-1)*mva[1] + (nb-1)*mvb[1]) / (na + nb - 2))
        #t= np.sqrt(mva[1]/na + mvb[1]/nb)
        t = (mva[0] - mvb[0]) / (pooled_std * np.sqrt(1/na + 1/nb))
        return t,mva,mvb


    def save_curves(self,
                    t,
                    mva,
                    mvb,
                    save_path,
                    time_tag='',
                    th_num=4.5,
                    show_window:Tuple =(None,None)
                    ):
        if show_window[0] and show_window[1]:
            t = t[0][show_window[0]:show_window[1]]
        else :
            t = t[0]

        x = np.array([i+show_window[0] for i in range(t.shape[0])])
        th_up = np.array([th_num for _ in range(t.shape[0])])
        th_down = np.array([-th_num for _ in range(t.shape[0])])
        style_kwargs_up = {'color': '#FF4500', 'zorder': 50, 'linewidth': 0.8, 'label': f'threshold={th_num}'}
        style_kwargs_down = {'color': '#B22222', 'zorder': 50, 'linewidth': 0.8, 'label': f'threshold=-{th_num}'}
        style_kwargs_t = {'color': '#2222a5', 'zorder': 50, 'linewidth': 0.8, 'label': 't'}
        
        plt.figure(figsize=(15,8))
        plt.plot(x,th_up,**style_kwargs_up)
        plt.plot(x,th_down,**style_kwargs_down)
        plt.plot(x,t,**style_kwargs_t)
        
        plt.title('TVLA Test Statistics')
        plt.xlabel('Sample Number')
        plt.xlim(x[0],x[-1])
        plt.ylabel('t')
        plt.ylim(-12,12)
        plt.legend()
        plt.grid(True)
        plt.axhline(0, color='black', linewidth=0.5)

        # 设置纵轴范围 (可根据需要调整)
        #plt.ylim(-6, 6)  # 例如：设置纵轴范围
        time_path = os.path.join(save_path,time_tag+'/')
        os.makedirs(time_path,exist_ok=True)
        fig_path = os.path.join(save_path,time_tag+'/', 'fig_tvla.png')
        plt.savefig(fig_path, dpi=300)
        print(f"[+] 图t已保存至: {fig_path}")
        plt.close()
        if show_window[0] and show_window[1]:
            ma ,va = mva[0][0][show_window[0]:show_window[1]], mva[1][0][show_window[0]:show_window[1]]
            mb ,vb = mvb[0][0][show_window[0]:show_window[1]], mvb[1][0][show_window[0]:show_window[1]]
        else:
            ma ,va = mva[0][0], mva[1][0]
            mb ,vb = mvb[0][0], mvb[1][0]
        
        k_ma = {'color': '#aa2200', 'zorder': 50, 'linewidth': 0.8, 'label': 'mean-A'}
        k_va = {'color': '#aa2233', 'zorder': 50, 'linewidth': 0.8, 'label': 'var-A'}
        k_mb = {'color': '#00aa22', 'zorder': 50, 'linewidth': 0.8, 'label': 'mean-B'}
        k_vb = {'color': '#33aa22', 'zorder': 50, 'linewidth': 0.8, 'label': 'var-B'}
        plt.figure(figsize=(15,8))
        plt.plot(x,ma,**k_ma)
        plt.plot(x,mb,**k_mb)
        plt.legend()
        plt.grid(True)
        plt.axhline(0, color='black', linewidth=0.5)
        fig_path = os.path.join(save_path,time_tag+'/', 'fig_mean.png')
        plt.savefig(fig_path, dpi=300)
        print(f"[+] 图mean已保存至: {fig_path}")
        plt.close()

        plt.figure(figsize=(15,8))
        plt.plot(x,va,**k_va)
        plt.plot(x,vb,**k_vb)
        plt.legend()
        plt.grid(True)
        plt.axhline(0, color='black', linewidth=0.5)
        fig_path = os.path.join(save_path,time_tag+'/', 'fig_var.png')
        plt.savefig(fig_path, dpi=300)
        print(f"[+] 图var已保存至: {fig_path}")
        plt.close()

if __name__ == "__main__":
    trace_file = os.path.join(DATA_ROOT,f"{special_b}{DIR_TAG}/averaged/{TRACE_PROCESS_MODE}/",TRACE_FILE_NAME)
    save_path = os.path.join(PICTURE_PATH,'tvla/')
    timestamp = datetime.now().strftime("%Y%m%d_%H:%M")
    trace_num = config_tvla.get("USE_TRACE_NUM")
    time_tag = timestamp + remark + f'-trace{trace_num}'
    show_window = (config_tvla.get("SHOW_START"),config_tvla.get("SHOW_END"))

    tvla = TVLA(
        tvla_trace_file=trace_file,
        traces_number= trace_num,
        sample_number=SAMPLE_NUM
    )
    tvla.read_traces()
    t,mva,mvb = tvla.get_tvla_result()
    tvla.save_curves(
        t,mva,mvb,
        save_path=save_path,
        time_tag=time_tag,
        show_window=show_window
        )