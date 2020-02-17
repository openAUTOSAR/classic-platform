import optparse
import re
import math

app = []
sec = []
bla = []

if __name__ == '__main__':
    parser = optparse.OptionParser()
    
    (options, args) = parser.parse_args()
    
    print args
    
    f = open(args[0],'r')

    # __OS_START_SEC_app_bss_<app>
    # __OS_START_SEC_app_bss_<app>
    for line in iter(f):
        m = re.match(r'.*(0x.*?)[ ]+__OS_(START|STOP)_SEC_(.*?)_(.*?)_(.*?) =.*',line)
        if (m):
            _addr = m.group(1)
            _addr_type = m.group(2)
            _group = m.group(3)
            _subgroup = m.group(4)
            _id = m.group(5)
            t=(_addr,_addr_type,_group,_subgroup,_id)
            sec.append(t)

    f.close()

    # sort it       
    sec_sorted = sorted(sec,key=lambda x: x[4]+x[3]+x[2]+x[1])

    for e in sec_sorted:
        if ("STOP" in e[1]):
            size = int(e[0],0) - int(start,0)            
            if (size != 0):
                q = math.pow(2,math.ceil(math.log((size*2 - 1),2)) - 1 )
            else:
                q  = 0 

            print "%20s %5s %10s %10s %6X %06X" % (e[4],e[3],start,e[0],size,q)
            if ((int(start,0)+q) != int(e[0],0)):
                print "[The above region is not aligned]"
        else:
            start=e[0]
        #print e
