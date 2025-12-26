
from datetime import datetime
from re import T
from typing import Tuple
import numpy as np
from multiprocessing import Pool, shared_memory, Process, Queue
from tqdm import tqdm as tq
import multiprocessing as mp
from datetime import datetime
import os
import time
from collections import defaultdict
import json
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
from DistanceModule import distance,get_plaintexts
from CpaAttack import column_pearson_corr



matplotlib.use('Agg')

plt.rcParams.update({
    'font.size': 15,                   # 默认字体大小
    'font.family': 'sans-serif',       # 字体系列
    'font.sans-serif': ['Times New Roman','DejaVu Sans', 'Arial'],  # 无衬线字体优先级
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
config_trace_num = config.get("TRACENUM",{})
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
KEY_WINDOW = (config_trace_num.get("KEY_START"),
              config_trace_num.get("KEY_END")
              )
SAMPLE_WINDOW = (config_trace_num.get("SAMPLE_START"),
                 config_trace_num.get("SAMPLE_END")
                )
STOP_PLAINTEXT_NUM = config_trace_num.get( "MAX_PLAINTEXT_NUM")
PICTURE_SHOW_NUM_START = config_trace_num.get("PICTURE_SHOW_NUM_START")
PICTURE_SHOW_NUM_END = config_trace_num.get("PICTURE_SHOW_NUM_END")
PICTURE_Y_WINDOW = (config_trace_num.get("PICTURE_Y_START"),config_trace_num.get("PICTURE_Y_END"))

RANDOM_FILE_NAME = general.get( "RANDOM_FILE_NAME")
RANDOM_FILE_PATH = path.get("SPECIAL_ROOT")
RANDOM_FILE = os.path.join(RANDOM_FILE_PATH,RANDOM_FILE_NAME)

PROCESS_NUM = config_trace_num.get("PROCESS_NUM")



high_contrast_colors = [ 
            '#B22222',  # 砖红色  
            '#8B0000',  # 深红色
            '#FFD700',  # 金黄色
            '#FF6347',  # 番茄红
            '#FF8C00',  # 深橙色
            '#FF4500',  # 橙红色
            '#FF1493',  # 深粉色
            '#FFA500',  # 橙色
            '#800000',  # 栗色
            '#FF4500',  # 橙红色
        ]

def incremental_pearson_corr(cumulative, new_power, new_h):
    """
    增量计算 Pearson 相关系数
    
    参数:
    cumulative - 累积统计量字典，包含:
        'n': 当前样本数量
        'sum_h': 中间值总和
        'sum_power': 功率轨迹总和 (向量)
        'sum_h_sq': 中间值平方和
        'sum_power_sq': 功率轨迹平方和 (向量)
        'sum_h_power': 中间值与功率轨迹乘积和 (向量)
        
    new_power - 新样本的功率轨迹 (向量)
    new_h - 新样本的中间值 (标量)
    
    返回:
    相关系数向量
    """
    cumulative['n']= cumulative['n'] + 1
    
    # 增量更新累积量
    cumulative['sum_h'][:] += new_h
    cumulative['sum_h_sq'][:] += new_h**2
    cumulative['sum_power'] += new_power
    cumulative['sum_power_sq'] += new_power**2
    cumulative['sum_h_power'] += new_h*new_power
    n = cumulative['n']
    a = cumulative['sum_h_power']
    bx,by = cumulative['sum_h_sq'], cumulative['sum_power_sq']
    cx,cy = cumulative['sum_h'],cumulative['sum_power']
    div_num = (n*bx - cx**2)*(n*by - cy**2)
    div_num[div_num<=0] = 1e-10
    # 计算相关系数
    corr = (n*a - cx*cy)/ np.sqrt(div_num)
    
    # 处理无效值
    corr = np.nan_to_num(corr, nan=0.0, posinf=0.0, neginf=0.0)
    
    return corr


def process_key_range(traces_shared_mem_info, plaintexts_shared_mem_info, keys, stop_trace_num, results_dir):
    """处理一组密钥"""
    traces_shm_name, traces_shape, traces_dtype = traces_shared_mem_info
    plaintext_shm_name, plaintext_shape, plaintext_dtype = plaintexts_shared_mem_info
    try:
        power_trace_shm = shared_memory.SharedMemory(name=traces_shm_name)
        plaintext_shm = shared_memory.SharedMemory(name=plaintext_shm_name)
        power_trace_mat = np.ndarray(traces_shape, dtype=traces_dtype, buffer=power_trace_shm.buf)
        plaintext_mat = np.ndarray(plaintext_shape, dtype=plaintext_dtype, buffer=plaintext_shm.buf)
        
        results = {}
        for key in keys:
            #初始化累积统计量
            cumulative = {
                'n': 0,
                'sum_h': np.zeros(traces_shape[1]),
                'sum_power': np.zeros(traces_shape[1]),
                'sum_h_sq': np.zeros(traces_shape[1]),
                'sum_power_sq': np.zeros(traces_shape[1]),
                'sum_h_power': np.zeros(traces_shape[1])
            }
            correlations = []
            H_distance_mat = np.zeros((stop_trace_num, power_trace_mat.shape[1]))
            for trace_num in range(0, stop_trace_num):
                power_data = power_trace_mat[trace_num]
                plaintexts = plaintext_mat[trace_num]
                h_val = distance(plaintexts, key)
                corr = incremental_pearson_corr(cumulative, power_data, h_val)
                #
                max_corr = np.max(corr)
                correlations.append(max_corr)
            
            results[key] = correlations
        
        # 批量保存结果到临时文件
        output_file = os.path.join(results_dir, f"partial_{keys[0]}_{keys[-1]}.json")
        with open(output_file, 'w') as f:
            json.dump(results, f)   
        return True
    
    finally:
        if 'existing_shm' in locals():
            power_trace_shm.close()

def merge_partial_results(results_dir, output_file, total_keys):
    """合并部分结果文件"""
    all_results = {}
    processed_keys = set()
    processed_num = 0
    total_num = total_keys
    for filename in os.listdir(results_dir):
        if filename.startswith('partial_'):
            filepath = os.path.join(results_dir, filename)
            with open(filepath, 'r') as f:
                partial_results = json.load(f)
                for key_str, correlations in partial_results.items():
                    key = int(key_str)
                    if key not in processed_keys:
                        all_results[key] = correlations
                        processed_keys.add(key)
                        processed_num += 1
                        if processed_num % 100 == 0:
                            print(f"\r processed:{processed_num}/{total_num}",end='')
    
    # 检查是否所有密钥都已处理
    if len(all_results) < total_keys:
        print(f"\n>>> 警告: 只有 {len(all_results)}/{total_keys} 个密钥被处理")
    
    # 按密钥排序并写入最终结果
    with open(output_file, 'w') as f:
        for key in sorted(all_results.keys()):
            corr_str = ",".join(f"{c:.4f}" for c in all_results[key])
            f.write(f"{key}:{corr_str}\n")
    
    print(f"\n>>> 写入最终结果: {len(all_results)} 个密钥")

class GetCpaTraceNum:
    def __init__(self, 
                power_trace_file, 
                rondom_file,
                save_path,
                special_b : int ,
                dir_tag: str = '',
                #create_data,
                sample_number: int  = 5000, 
                stop_plaintext_number: int =3329, 
                key_window : Tuple =(0,3329),
                sample_window : Tuple = (0,3329),
                process_number : int = None, 
                plaintext_list_size : int = 6,
                down_factor: int  = 20,
                ):
        # files
        self.power_trace_file = power_trace_file
        self.rondom_file = rondom_file
        if not os.path.isfile(self.power_trace_file):
            raise ValueError(f"Cant find file:{self.power_trace_file}")
        if not os.path.isfile(self.rondom_file):
            raise ValueError(f"Cant find file:{self.rondom_file}")
        self.save_path = save_path
         # samples
        self.down_factor = down_factor
        # result save
        filename = os.path.splitext(os.path.basename(self.power_trace_file))[0]
        root_path =os.path.join(save_path,f'{special_b}{dir_tag}_{filename}' + '/') 
        data_dir_str = f'MP{stop_plaintext_number}K{key_window[0]}_{key_window[1]}S{sample_window[0]}_{sample_window[1]}D{down_factor}/'
        self.result_data_path = os.path.join(root_path,data_dir_str)
        self.result_file_name = "corrs.txt"
        self.result_file = os.path.join(self.result_data_path,self.result_file_name)
        self.save_picture_path = os.path.join(self.result_data_path,'pictures/')

        os.makedirs(self.result_data_path, exist_ok=True)
        os.makedirs(self.save_picture_path,exist_ok=True)
       
        # guess keys
        self.key_start = key_window[0]
        self.key_end   = key_window[1]
        self.key_number = key_window[1] - key_window[0]
        # plaintexts
        self.stop_plaintext_number = stop_plaintext_number
        self.plaintext_list_size = plaintext_list_size
        # processes
        self.process_number = process_number or max(1, mp.cpu_count() - 2)
        
        # 自动适配CPU核心数//max 48
        max_processes = min(48, os.cpu_count() * 2)
        self.process_number = min(self.process_number, max_processes)
        
        if sample_window[0] and sample_window[1]:
            self.low_sample = sample_window[0]
            self.high_sample = sample_window[1]
        else:
            self.low_sample = 0
            self.high_sample = sample_number
        if self.high_sample > sample_number or self.high_sample < self.low_sample:
            raise ValueError(f"Error sample window:({self.low_sample},{self.high_sample})")
                    
        self.sample_number = (self.high_sample - self.low_sample)//self.down_factor
        self.power_trace_mat = None
        self.plaitext_mat = None

    def read_power(self):
        if os.path.isfile(self.result_file):
            print("INFO: result file exists. skip to read power.")
            return
        """高效读取功率轨迹数据"""
        print(f">>> 读取功率轨迹数据 (样本范围: {self.low_sample}-{self.high_sample})")
        
        # 初始化功率矩阵
        self.power_trace_mat = np.zeros((self.stop_plaintext_number, self.sample_number), dtype=np.float32)
        # plaintexts
        self.plaitext_mat = np.zeros((self.stop_plaintext_number,self.plaintext_list_size),dtype=np.int32)
        
        # 使用缓冲区减少内存分配次数
        buffer_size = 1000
        buffer = []
        
        number_of_plaintext = 0
        read_trace_num = 0
        with tq(total=self.stop_plaintext_number , desc=">>> 读取功率轨迹") as bar:
            with open(self.power_trace_file, 'r') as pf:
                for line in pf:
                    if not line.strip():
                        continue
                    else:
                        parts = line.split(':', 1)
                        if len(parts) < 2:
                            continue
                        #plaintext_str, power_trace_str = parts
                        #plaintext = int(plaintext_str)
                        _, power_trace_str = parts
                        # 
                        if 0 <= read_trace_num < self.stop_plaintext_number:
                            power_trace = np.fromstring(power_trace_str, sep=' ', dtype=np.float32)
                            # 应用样本范围
                            power_trace = power_trace[self.low_sample:self.high_sample].reshape(
                                                            1,
                                                            self.sample_number, 
                                                            self.down_factor
                                                            ).max(axis=2)

                            plaintexts = get_plaintexts(
                                file_path=self.rondom_file,
                                trace_number=read_trace_num,
                                plaintext_num=self.plaintext_list_size
                                )
                            buffer.append((plaintexts, power_trace))
                            
                            # 缓冲区满时批量处理
                            if len(buffer) >= buffer_size:
                                for pl, trace in buffer:
                                    self.power_trace_mat[number_of_plaintext] = trace
                                    self.plaitext_mat[number_of_plaintext] = pl
                                    number_of_plaintext += 1
                                buffer = []
                            bar.update(1)
                            read_trace_num += 1
                            if read_trace_num >= self.stop_plaintext_number:
                                break
        
        # 处理剩余数据
        if buffer:
            for pl, trace in buffer:
                self.power_trace_mat[number_of_plaintext] = trace
                self.plaitext_mat[number_of_plaintext] = pl
                number_of_plaintext +=1
        
        print(f">>> 成功读取 {number_of_plaintext}/{self.stop_plaintext_number} 条功率轨迹")

    def process_all_traces(self, stop_plaintext_num=None,max_traces = 1000000):
        if os.path.isfile(self.result_file):
            print("INFO: result file exists. skip to calculate CPA.")
            return
        if stop_plaintext_num is None:
            stop_plaintext_num = min(self.stop_plaintext_number,max_traces)  # 限制最大轨迹数
 
        print(f">>> 开始分析轨迹数量和密钥相关性 (轨迹数: 1-{stop_plaintext_num})")
        print(f">>> 密钥:{self.key_start}-{self.key_end} total:{self.key_number} | 进程数: {self.process_number}")
        
        # 创建共享内存存放能量迹矩阵
        start_time = time.time()
        traces_shm = shared_memory.SharedMemory(create=True, size=self.power_trace_mat.nbytes)
        plaintexts_shm = shared_memory.SharedMemory(create=True, size=self.plaitext_mat.nbytes)
        shared_trace_mat = np.ndarray(
            self.power_trace_mat.shape, 
            dtype=self.power_trace_mat.dtype, 
            buffer=traces_shm.buf
        )
        shared_plaintext_mat = np.ndarray(
            self.plaitext_mat.shape, 
            dtype=self.plaitext_mat.dtype, 
            buffer=plaintexts_shm.buf
        )
        np.copyto(shared_trace_mat, self.power_trace_mat)
        np.copyto(shared_plaintext_mat,self.plaitext_mat)
        print(f">>> 共享内存创建完成 (耗时: {time.time()-start_time:.2f}s)")
        
        # 准备进程池
        traces_shared_mem_info = (traces_shm.name, self.power_trace_mat.shape, self.power_trace_mat.dtype)
        plaintexts_shared_mem_info = (plaintexts_shm.name, self.plaitext_mat.shape, self.plaitext_mat.dtype)
        # 分批处理密钥以减少内存压力
        chunk_size = min(100, max(10, self.key_number // (self.process_number * 2)))
        key_ranges = []
        for i in range(self.key_start, self.key_end, chunk_size):
            end = min(i + chunk_size, self.key_end)
            if i != self.key_end:
                key_ranges.append((i, end))
        
        print(f">>> 密钥分块: {len(key_ranges)} 块 | 每块大小: {chunk_size} 密钥")
        
        # 使用进程池处理密钥
        results_dir =os.path.join(self.save_path, "tmp_results")
        os.makedirs(results_dir, exist_ok=True)
        
        with Pool(processes=self.process_number) as pool:
            futures = []
            # 提交任务
            for start_key, end_key in key_ranges:
                keys = list(range(start_key, end_key))
                future = pool.apply_async(
                    process_key_range, 
                    (traces_shared_mem_info,plaintexts_shared_mem_info, keys, stop_plaintext_num, results_dir)
                )
                futures.append(future)
            
            # 等待所有任务完成
            with tq(total=len(futures), desc=">>> 处理密钥批次") as pbar:
                for future in futures:
                    future.get()  # 等待完成
                    pbar.update(1)
        
        # 合并部分结果
        print(">>> 合并部分结果...")
        output_file = self.result_file
        merge_partial_results(results_dir, output_file, self.key_number)
        
        # 清理共享内存
        traces_shm.close()
        plaintexts_shm.close()
        traces_shm.unlink()
        plaintexts_shm.unlink()
        
        # 清理临时文件
        for f in os.listdir(results_dir):
            os.remove(os.path.join(results_dir, f))
        os.rmdir(results_dir)
        
        print(f">>> 分析完成! 结果保存至: {output_file}")
        print(f">>> 总耗时: {time.time()-start_time:.2f}秒")
    
    def show_traces(self,
                    picture_tag='',
                    highlight_keys=None,
                    start_plaintext_number=3,
                    stop_plaintext_number=3329,
                    save_picture=True,
                    y_window = (0,0.5),
                    x_step = 20):
        result_file = self.result_file
        if not os.path.isfile(result_file):
            raise ValueError(f'Need correlations result file: {result_file}.')
        # 初始化功率矩阵
        #corr_curves = np.zeros((self.key_number, stop_plaintext_number), dtype=np.float32)
        corr_curves = {}
        # 使用缓冲区减少内存分配次数
        buffer_size = 1000
        buffer = []
        current_index = 0
        with tq(total=self.key_number, desc="读取相关系数曲线") as bar:
            with open(result_file, 'r') as pf:
                for line in pf:
                    if not line.strip():
                        continue
                    parts = line.split(':', 1)
                    if len(parts) < 2:
                        continue
                    key_str, correlation_str = parts
                    key = int(key_str)
                    
                    # 只处理在范围内的明文 [a,b)
                    if self.key_start <= key < self.key_end:
                        correlations = np.fromstring(correlation_str, sep=',', dtype=np.float32)
                        
                        # 应用样本范围
                        buffer.append((key, correlations))
                        # 缓冲区满时批量处理
                        if len(buffer) >= buffer_size:
                            for key, corrs in buffer:
                                corr_curves[key] = corrs
                            buffer = []
                        bar.update(1)
                        current_index += 1
                        
                        if current_index >= self.key_number:
                            break   
        
        # 处理剩余数据
        if buffer:
            for key, corrs in buffer:
                corr_curves[key] = corrs
        
        print(f">>> 成功读取 {self.key_number} 条相关系数曲线")
        print(">>> 准备可视化结果...")
        
        # 获取所有密钥的相关系数数据
        # 从第4个数据开始
        all_corrs = np.array([corr_curves[key][start_plaintext_number:stop_plaintext_number] for key in range(self.key_start,self.key_end)])
        print('Data read finish')
        # 创建图形和坐标轴
        plt.figure(figsize=(15, 8))
        # 否则只创建单个视图
        ax = plt.subplot(1, 1, 1)

        # 绘制所有密钥的相关系数曲线 (高性能方式)
        # 使用透明浅色绘制所有曲线
        x = np.arange(start_plaintext_number,stop_plaintext_number)*x_step
        print(">>> Start to draw traces")
        for i in range(0,len(all_corrs),100):
            end = min(len(all_corrs)-1,i+100)
            segments = np.array([np.column_stack([x, y]) for y in all_corrs[i:end]])
            norm = plt.Normalize(0, len(all_corrs))
            lc = LineCollection(segments, cmap='Greys', norm=norm, alpha=0.3, linewidth=0.7)
            lc.set_color('gray')
            ax.add_collection(lc)
            print(f"\r drawing:{i}/{len(all_corrs)}",end='')

        # 设置坐标轴范围
        ax.set_xlim(start_plaintext_number*x_step, stop_plaintext_number*x_step)
        ax.set_ylim(y_window[0], y_window[1])  # 相关系数范围
        #ax.set_ylim(-0.5, 0.35)  # 相关系数范围

        # 添加网格
        ax.grid(True, linestyle='--', alpha=0.6)

        # 添加标签
        ax.set_xlabel('Trace number')
        ax.set_ylabel('Correlation')
        # 创建高对比度颜色列表（避免蓝色）
        print("\n>> draw hilight keys")
        # 突出显示特定密钥
        if highlight_keys:
            print(f"highlight key: {highlight_keys}")
            #colors = plt.cm.tab10(np.linspace(0, 1, len(highlight_keys)))
            for i, key in enumerate(highlight_keys):
                corrs = corr_curves[key][start_plaintext_number:stop_plaintext_number].flatten()
                label = f'key {key}'
                ax.plot(x,corrs, color=high_contrast_colors[i%10], linewidth=1, alpha=0.9, label=label)
            # 添加图例
            ax.legend(loc='upper right')

        # 添加标题
        #title = f'CPA result per Trace number ({self.key_start}-{self.key_end},total {self.key_number} keys)'
        #title = f'CPA result per Trace number '
        title = f'Correlation-Trace number '
        # if highlight_keys:
        #     title += f'\nhighlight key(s): {", ".join(map(str, highlight_keys))}'
        plt.suptitle(title, fontsize=14)
        plt.tight_layout()
        picture_file = os.path.join(
            self.save_picture_path,
            f"fig-traces{start_plaintext_number}-{stop_plaintext_number}-y{y_window[0]}-{y_window[1]}-{picture_tag}.png"
            )
        
        if save_picture:
            print(">> Start to save picture")
            plt.savefig(
                picture_file, 
                dpi=300,
                format='png',                # 明确指定格式
                bbox_inches='tight',         # 裁剪空白，减小文件大小
                pad_inches=0.05,             # 减少内边距
                facecolor='white',           # 明确指定背景色
                edgecolor='none',            # 去除边缘颜色
                )
            print(f">> Save picture to : {picture_file}")
        else:
            plt.show()


if __name__ == "__main__":
    power_trace_file =  os.path.join(DATA_ROOT,f"{special_b}{DIR_TAG}/averaged/{TRACE_PROCESS_MODE}/",TRACE_FILE_NAME)
    save_path = os.path.join(PICTURE_PATH,'trace_num/')
    get_trace_num = GetCpaTraceNum(
        power_trace_file=power_trace_file,
        rondom_file=RANDOM_FILE,
        save_path=save_path,
        special_b=special_b,
        dir_tag=DIR_TAG,
        sample_number=SAMPLE_NUM,
        stop_plaintext_number=STOP_PLAINTEXT_NUM,
        key_window=KEY_WINDOW,
        sample_window=SAMPLE_WINDOW,
        process_number=PROCESS_NUM
    )
    get_trace_num.read_power()
    get_trace_num.process_all_traces()
    get_trace_num.show_traces(
        picture_tag=remark,
        highlight_keys=[special_b],
        start_plaintext_number=PICTURE_SHOW_NUM_START,
        stop_plaintext_number=PICTURE_SHOW_NUM_END,
        y_window=PICTURE_Y_WINDOW
    )