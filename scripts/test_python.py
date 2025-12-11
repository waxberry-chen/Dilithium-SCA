import time 
import numpy as np
import cupy as cp
from CpaAttack import distance
from CpaAttack import column_pearson_corr

# a = cp.random.rand(10, 10)
# b = cp.random.rand(10, 10)
# cp.dot(a, b)  # 触发编译

# # 生成数据
# numpy_vec = np.random.rand(1, 300000).astype(np.float32)
# numpy_mat = np.random.rand(300000, 1000).astype(np.float32)

# cupy_vec = cp.array(numpy_vec)  # 复制数据到 GPU
# cupy_mat = cp.array(numpy_mat)

# # NumPy 计算 (CPU)
# start = time.time()
# numpy_result = numpy_vec @ numpy_mat
# numpy_time = time.time() - start
# print(f"NumPy 耗时: {numpy_time:.5f} 秒")

# # CuPy 计算 (GPU)
# start = time.time()
# cupy_result = cupy_vec @ cupy_mat
# cp.cuda.Stream.null.synchronize()  # 等待 GPU 完成
# cupy_time = time.time() - start
# print(f"CuPy 耗时: {cupy_time:.5f} 秒")

# # 加速比
# print(f"\n加速比: NumPy / CuPy = {numpy_time / cupy_time:.1f}x")
# RANDOM_FILE = "/15T/Projects/Dilithium-SCA/data/special_files/Random_3000.txt"

# import linecache

# def get_plaintexts(file_path,trace_number,plaintext_num=6):
#     plaintexts = []
#     for i in range(plaintext_num):
#         line = linecache.getline(file_path, trace_number+i+1).rstrip('\n')#从1开始
#         if not line:
#             raise ValueError("Plaintexts file line num not enough")
#         plaintexts.append(int(line)) 
#     return plaintexts  

# for i in range(5):
#   print(get_plaintexts(RANDOM_FILE,i))

import matplotlib
import matplotlib.pyplot as plt
import linecache
import numpy as np
import os
matplotlib.use('Agg')

def HD(num_before,num_after):
    xor_result = num_before^num_after
    total = bin(xor_result).count('1')
    
    LH =bin(xor_result&num_after).count('1')
    HL = total - LH
    return LH,HL

def HD_all(num_before,num_after):
    LH,HL = HD(num_before,num_after)
    return 1*HL + 1* LH

def get_trace(file_path,plaintext_num):
    line = linecache.getline(file_path,plaintext_num+1) # 读取第plaintext行的数据（0->plaintext_num-1）
    if not line:
        raise ValueError("ERROR: file error ")
    _, trace_str = line.strip().split(':',1)
    trace = np.array(trace_str.strip().split()).astype(np.float64)
    return trace

def get_plaintexts(file_path,trace_number,plaintext_num=6):
    plaintexts = []
    for i in range(plaintext_num):
        line = linecache.getline(file_path, trace_number+i+1).rstrip('\n') #从1开始(HERE)
        if not line or line.isspace():
            raise ValueError(f"Plaintexts file line num not enough or cant find file {file_path}")
        plaintexts.append(int(line)) 
    line = linecache.getline(file_path,trace_number+4)
    plaintext_last = 0
    if not line:
        plaintext_last = int(line) if trace_number >0 else 0
    plaintexts.append(plaintext_last)
    return plaintexts  


q=3329
def calculate_sim(plaintexts,key):
    sim_trace = [0 for _ in range(6)]
    hd_a = 0
    hd_p0,hd_p0d = 0,0
    hd_mm = 0
    hd_o =0
    for wave_num in range(2,7):
        hd_a = HD_all(plaintexts[1+wave_num-2],plaintexts[2+wave_num-2])
        hd_p0 = HD_all(plaintexts[0+wave_num-2]*key,plaintexts[1+wave_num-2]*key)
        #p0d
        if wave_num ==2:
            hd_p0d = HD_all(plaintexts[6]*key&(2**25-1),plaintexts[0]*key&(2**25-1))
        else :
            hd_p0d += HD_all(plaintexts[0+wave_num-2]*key&(2**25-1),plaintexts[1+wave_num-1]*key&(2**25-1))
        # hd_mm 
        if wave_num ==5:
            hd_mm = HD_all(plaintexts[6]*key%q,plaintexts[0]*key%q)
        elif wave_num == 6:
            hd_mm = HD_all(plaintexts[0]*key%q,plaintexts[1]*key%q)
            hd_o = HD_all(plaintexts[6]*key%q,plaintexts[0]*key%q)
        sim_trace[wave_num-2] = hd_a + hd_p0 + hd_p0d + hd_mm + hd_o
    return sim_trace
        

def calculate_act(file_path,plaintext_num):
    act_trace = get_trace(file_path=file_path,plaintext_num=plaintext_num)
    num_shift = 1000
    position = []
    for i in range(5):
        position.append(i*num_shift+np.argmax(act_trace[ i*num_shift : (i+1)*num_shift-1 ]))
    print(f'positions : {position}')
    return act_trace,position


def get_sim_actual(file_trace,file_random,trace_num,key=2773):
    plaintexts = get_plaintexts(file_path=file_random,trace_number=trace_num)
    sim_trace = calculate_sim(plaintexts=plaintexts,key=key)
    act_trace ,position = calculate_act(file_path=file_trace,plaintext_num=trace_num)
    return act_trace,sim_trace,position

def draw_trace(act_trace,sim_trace,position,x_num = 5000):
    x = [i for i in range(x_num)]
    y1 = act_trace
    y2 = [0 for _ in range(x_num)]
    for i,index in enumerate(position):
        y2[index] = sim_trace[i]
    plt.figure(1)
    plt.subplot(2,1,1)
    plt.plot(x,y1)
    plt.subplot(2,1,2)
    plt.plot(x,y2)
    save_path = '/15T/Projects/Dilithium-SCA/result/test.png'
    plt.savefig(save_path,dpi=300)

a_last = 0

def regs_transfer_count(plaintexts,key):
    global a_last
    mask = 0x1ffff
    p0_d2 = HD_all(plaintexts[1]*key&mask,plaintexts[2]*key&mask)
    p0_d1 = HD_all(plaintexts[2]*key&mask,plaintexts[3]*key&mask)
    p0_d0 = HD_all(plaintexts[3]*key&mask,plaintexts[4]*key&mask)
    a_last=plaintexts[5]
    return p0_d0,p0_d1,p0_d2

if __name__ == "__main__":
    # file_trace = '/15T/Projects/Dilithium-SCA/data/traces/2773_kyber_q3329/averaged/align/averaged_mau_loop20.txt'
    # if not os.path.isfile(file_trace):
    #     raise ValueError("file path wrong")
    # file_random = '/15T/Projects/Dilithium-SCA/data/special_files/random_3000_0-3328.txt'
    # if not os.path.isfile(file_random):
    #     raise ValueError("file path wrong")
    
    # trace_act,trace_sim,position = get_sim_actual(file_trace=file_trace,file_random=file_random,trace_num=2001)
    # draw_trace(act_trace=trace_act,sim_trace=trace_sim,position=position)

    # print(get_plaintexts(file_path=file_random,trace_number=0))
    # for i in range(144):
    #     print(2**(i%24)-1)
    # m= 5039
    # n=24
    # p=3329

    # test_a = [123,333,555,666,777]
    # test_b = 2773

    # def cal_wt_xor(a,b):
    #     p = a*b
    #     p_temp = p - ((p*m)>>n)*q
    #     if p_temp > p:
    #         p = p_temp - p
    #     else :
    #         p = p_temp
    #     return p

    # def cal_xor(a,b):
    #     v=0xabcd
    #     p=a*b^v
    #     p_temp = p - (p*m>>n)*q
    #     if p_temp > p:
    #         p = p_temp - p
    #     else :
    #         p = p_temp
    #     return p^v
    
    # def cal(a,b):
    #     return a*b%q

    # for a in test_a:
    #     p1 = cal(a,test_b)
    #     p2 = cal_wt_xor(a,test_b)
    #     p3 = cal_xor(a,test_b)
    #     print(f'a:{a},b:{test_b}, cal:{p1},cal with out xor:{p2},cal with xor:{p3}')

    ############# test regs transfer
    # file_random = '/15T/Projects/Dilithium-SCA/data/special_files/random_3000_0-3328.txt'
    # save_path = "/15T/Projects/Dilithium-SCA/result/test/"
    # key = 1037
    # d0_list,d1_list,d2_list,all_list = [],[],[],[]
    # mm_list = []
    # number = 2994
    # for i in range(number):
    #     plaitexts = get_plaintexts(file_path=file_random,trace_number=i)
    #     p0_d0,p0_d1,p0_d2 = regs_transfer_count(plaintexts=plaitexts,key=key)
    #     d0_list.append(p0_d0)
    #     d1_list.append(p0_d1)
    #     d2_list.append(p0_d2)
    #     all_list.append(p0_d0+p0_d1+p0_d2)
    #     mm = distance(plaintexts=plaitexts,key=key)
    #     mm_list.append(mm)
    # rall = np.corrcoef(np.array(mm_list),np.array(all_list))[0,1]
    # print(f"mm/all: {rall}")
    # rd0 = np.corrcoef(np.array(mm_list),np.array(d0_list))[0,1]
    # print(f"mm/d0: {rd0}")
    # rd1 = np.corrcoef(np.array(mm_list),np.array(d1_list))[0,1]
    # print(f"mm/d1: {rd1}")
    # rd2 = np.corrcoef(np.array(mm_list),np.array(d2_list))[0,1]
    # print(f"mm/d2: {rd2}")
    # x = [i for i in range(number)]
    # plt.figure(0)
    # plt.plot(x,mm_list,label="mm")
    # plt.plot(x,d0_list,label="d0")
    # plt.plot(x,d1_list,label="d1")
    # plt.plot(x,d2_list,label="d2")
    # plt.plot(x,all_list,label="d0+d1+d2")
    # plt.legend()
    # plt.savefig(save_path+'regs_transfer.png')
    # import random
    # for i in range(10):
    #     a = random.randint(0,1000)
    #     b = random.randint(0,1000)
    #     a_mask = random.randint(0,1000)
    #     left = (a^a_mask)*(b^a_mask)
    #     right = (a*b)^(b*a_mask)^(a*a_mask)^(a_mask*a_mask)
    #     print(f"left:{left},right:{right}")
    # a = [[1,2,3],[1,2,3]]
    # npa = np.array(a)
    # print(npa.shape[1])
    help(np.reshape)
  