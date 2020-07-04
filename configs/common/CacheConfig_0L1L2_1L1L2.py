from __future__ import print_function

import m5
from m5.objects import *
from Caches import *

def config_cache(options, system):
    if options.external_memory_system and (options.caches or options.l2cache):
        print("External caches and internal caches are exclusive options.\n")
        sys.exit(1)

    if options.external_memory_system:
        ExternalCache = ExternalCacheFactory(options.external_memory_system)

    if options.cpu_type == "O3_ARM_v7a_3":
        try:
            from cores.arm.O3_ARM_v7a import *
        except:
            print("O3_ARM_v7a_3 is unavailable. Did you compile the O3 model?")
            sys.exit(1)

        dcache_class, icache_class, l2_cache_class, walk_cache_class = \
            O3_ARM_v7a_DCache, O3_ARM_v7a_ICache, O3_ARM_v7aL2, \
            O3_ARM_v7aWalkCache
    else:
        dcache_class, icache_class, l2_cache_class, walk_cache_class = \
            L1_DCache, L1_ICache, L2Cache, None

        if buildEnv['TARGET_ISA'] == 'x86':
            walk_cache_class = PageTableWalkerCache
            
    system.cache_line_size = options.cacheline_size
    
    if options.l2cache and options.elastic_trace_en:
        fatal("When elastic trace is enabled, do not configure L2 caches.")
    if options.memchecker:
        system.memchecker = MemChecker()
    
    #Adding L1 i and d cache for CPU 0 and CPU 1    
    if options.caches: 
        #For CPU 0
        icache_cpu0 = icache_class(size=options.l1i_size,
                              assoc=options.l1i_assoc)
        dcache_cpu0 = dcache_class(size=options.l1d_size,
                              assoc=options.l1d_assoc)
        if walk_cache_class:
            iwalkcache_cpu0 = walk_cache_class()
            dwalkcache_cpu0 = walk_cache_class()
        else:
            iwalkcache_cpu0 = None
            dwalkcache_cpu0 = None
        system.cpu[0].addPrivateSplitL1Caches(icache_cpu0, dcache_cpu0,
                                              iwalkcache_cpu0, dwalkcache_cpu0)

        #For CPU 1
        icache_cpu1 = icache_class(size=options.l1i_size,
                              assoc=options.l1i_assoc)
        dcache_cpu1 = dcache_class(size=options.l1d_size,
                              assoc=options.l1d_assoc)
        if walk_cache_class:
            iwalkcache_cpu1 = walk_cache_class()
            dwalkcache_cpu1 = walk_cache_class()
        else:
            iwalkcache_cpu1 = None
            dwalkcache_cpu1 = None
        system.cpu[1].addPrivateSplitL1Caches(icache_cpu1, dcache_cpu1,
                                              iwalkcache_cpu1, dwalkcache_cpu1)



    #Create interrupts for both the CPU 1 and 2                                          
    system.cpu[0].createInterruptController()
    system.cpu[1].createInterruptController()
    
    #Adding l2 cache for CPU 0 and 1 
    if options.l2cache:
        system.l2_cpu0 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l2_size,
                                   assoc=options.l2_assoc)
        system.tol2_cpu0bus = L2XBar(clk_domain = system.cpu_clk_domain)
        system.l2_cpu0.cpu_side = system.tol2_cpu0bus.master
        system.l2_cpu0.mem_side = system.membus.slave
        system.cpu[0].connectAllPorts(system.tol2_cpu0bus, system.membus)

        system.l2_cpu1 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l2_size,
                                   assoc=options.l2_assoc)
        system.tol2_cpu1bus = L2XBar(clk_domain = system.cpu_clk_domain)
        system.l2_cpu1.cpu_side = system.tol2_cpu1bus.master
        system.l2_cpu1.mem_side = system.membus.slave
        system.cpu[1].connectAllPorts(system.tol2_cpu1bus, system.membus)
    elif options.external_memory_system:
        system.cpu[0].connectUncachedPorts(system.membus)
    else:
        system.cpu[0].connectAllPorts(system.membus)
    #since there is no l2 chache for cpu 1 connect all the ports of membus directly to cpu    
    # system.cpu[1].connectAllPorts(system.membus)
    return system
    
class ExternalCache(ExternalSlave):
    def __getattr__(cls, attr):
        if (attr == "cpu_side"):
            attr = "port"
        return super(ExternalSlave, cls).__getattr__(attr)

    def __setattr__(cls, attr, value):
        if (attr == "cpu_side"):
            attr = "port"
        return super(ExternalSlave, cls).__setattr__(attr, value)

def ExternalCacheFactory(port_type):
    def make(name):
        return ExternalCache(port_data=name, port_type=port_type,
                             addr_ranges=[AllMemory])
    return make
