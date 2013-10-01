from math import floor, log

def newfactgmpy(n,v=0):
    logn = (int(round(floor(log(n,10)))))**6
    print logn     
    if v == 1:
    	print logn
    multitotals = []
    for i in xrange(0,logn):
        multitotals.append(gmpy.mpz(1));
    lf = n // (logn-1)
    frs = [1]
    for i in xrange(0,logn-2):
        frs.append(lf*(i+1))
    frs.append(n)
    if v == 1:
        print "beginning initial calculation"
        print len(frs)
    for i in xrange(1,len(frs)):
        
        if v == 1:
        	print i
        	print frs[i]
        for j in xrange(frs[i-1],frs[i]):
            
            multitotals[i] = multitotals[i] * gmpy.mpz(j)
    if v == 1:
    	print len(multitotals)
    totals = []
    total = gmpy.mpz(1)
    while len(multitotals) > 1:
        for i in xrange(1,len(multitotals),2):
            totals.append(multitotals[i-1] * multitotals[i])
        if v == 1:
            print len(multitotals)
        multitotals = totals
        totals = []
    return multitotals[0]
    
print newfactgmpy (1000)
