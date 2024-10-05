from subprocess import call
import numpy as np
import matplotlib.pyplot as plt

def parse(name: str) -> tuple[int, int, int]:
    f = open(name, 'r')
    nums_str= f.read().split()[0:3]
    # close as soon as possible
    f.close()
    return [float(x) for x in nums_str]
##    print(nums)
# %E\t %U\t %S \t

if __name__ == "__main__":
    filename = "timer_output.txt"
    x = 1
    x_axis = []
    out_wall = []
    out_user = []
    out_sys = []
    while x < 65536:
        print(x)
        x_axis.append(x)
        call(["/usr/bin/time", "--output=timer_output.txt", "-f", r"%e\t%U\t%S\t", './kit', '-b', str(x), '-o', 'testfile_out', 'testfile_64'])
        outs = parse('timer_output.txt')
        out_wall.append(outs[0])
        out_user.append(outs[1])
        out_sys.append(outs[2])
        x = x * 2       

    out_wall = np.array(out_wall,dtype=np.float32)
    out_sys = np.array(out_sys,dtype=np.float32)
    out_user= np.array(out_user,dtype=np.float32)
    x_axis = np.array(x_axis)
    print(x_axis.shape)
    fig, ax = plt.subplots()
    ax.set_xscale('log', base=2)
    ax.plot(x_axis, out_wall,color="red",label='wall time')
    ax.plot(x_axis, out_user,color="blue",label='user time')
    ax.plot(x_axis, x_axis,color="green",label='sys time')
    ax.legend(loc="upper left")
    fig.savefig('graph.pdf')
