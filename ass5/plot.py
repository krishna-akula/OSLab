import matplotlib.pyplot as plt

def main():
    try:
        with open('input.txt','r') as f:
            lines = f.readlines()
        lines=[i.strip() for i in lines]
        lines = [i.split()[-1] for i in lines ]
        lines=map(int,lines)
        print len(lines)
        plt.xlabel("page Number referenced")
        plt.ylabel("instruction number")
        # plt.axis([0,len(lines),-1,max(lines)])
        plt.scatter(lines,xrange(len(lines)))
        plt.savefig("results.jpeg")
        plt.show()
    except:
        pass

if __name__=='__main__':
    main()