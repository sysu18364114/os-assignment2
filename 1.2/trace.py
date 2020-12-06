
## 2020/12/2 ##

import matplotlib.pyplot as plt
import random
import os

FILE_ROOT = 'E:/OneDrive/VSCode_Python/projects/os_assignment_2/'
YIELD_NUM=10000
MEM_SIZE = 1000

def plot_trace(addr_list, filename):
    """
    绘制内存访问分布图
    参数：
        addr_list: 存储访问内存地址的列表
        filename: 希望的输出图片的文件名
    返回值：
        无返回值
    """
    plt.figure()
    list_x = [x for x in range(1, len(addr_list) + 1)] # 构造x轴坐标
    plt.plot(list_x, addr_list) # 绘制图形
    plt.title("Trace of Memory Reference")
    plt.xlabel("X Axis - Time Series")
    plt.ylabel("Y Axis - Virtual Memory Address")
    plt.savefig(''.join([FILE_ROOT, filename]))
    plt.show()

if __name__ == "__main__":
    addr_ref = []
    with open(''.join([FILE_ROOT, 'addresses.txt']), 'r') as fp:
        lines = fp.readlines()
        for ii in range(len(lines)):
            addr_ref.append(int(lines[ii]))
    
    # 模拟内存访问的局部性生成访问地址
    addr_list = []
    print(min(addr_ref), max(addr_ref))
    with open(''.join([FILE_ROOT, 'addresses-locality.txt']), 'w') as fp:
        for ii in range(int(YIELD_NUM/MEM_SIZE)):
            anchor = random.randint(0, max(addr_ref))
            for jj in range(MEM_SIZE):
                tmp_addr = random.randint(anchor-500, anchor+500) # 在锚点的正负500区间生成地址
                addr_list.append(tmp_addr)
                fp.write(''.join([str(tmp_addr), '\n']))
    plot_trace(addr_list, 'locality_plot.jpg'); # 绘制内存访问分布图

    # 随机生成访问地址
    addr_list = []
    with open(''.join([FILE_ROOT, 'addresses-random.txt']), 'w') as fp:
        for ii in range(YIELD_NUM):
            addr_tmp = random.randint(min(addr_ref), max(addr_ref))
            addr_list.append(addr_tmp)
            fp.write(''.join([str(addr_tmp),'\n']))
    plot_trace(addr_list, 'random_plot.jpg'); # 绘制内存访问分布图
    
    