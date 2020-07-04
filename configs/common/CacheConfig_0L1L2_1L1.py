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

    #Adding L1 i and d cache for CPU 1 	                                                
    if options.caches: #for CPU 1 L1I and L1D caches
        icache = icache_class(size=options.l1i_size,
                              assoc=options.l1i_assoc)
        dcache = dcache_class(size=options.l1d_size,
                              assoc=options.l1d_assoc)
        if walk_cache_class:
            iwalkcache = walk_cache_class()
            dwalkcache = walk_cache_class()
        else:
            iwalkcache = None
            dwalkcache = None
        system.cpu[1].addPrivateSplitL1Caches(icache, dcache,
                                              iwalkcache, dwalkcache)
        system.cpu[1].createInterruptController()                                      
        system.cpu[1].connectAllPorts(system.membus)
      
    #Adding L1 i and d cache for CPU 0  
    if options.caches: #for CPU 0 L1I and L1D caches
        icache = icache_class(size=options.l1i_size,
                              assoc=options.l1i_assoc)
        dcache = dcache_class(size=options.l1d_size,
                              assoc=options.l1d_assoc)
        if walk_cache_class:
            iwalkcache = walk_cache_class()
            dwalkcache = walk_cache_class()
        else:
            iwalkcache = None
            dwalkcache = None
        system.cpu[0].addPrivateSplitL1Caches(icache, dcache,
                                              iwalkcache, dwalkcache) 	                                                
    system.cpu[0].createInterruptController()
    
    #Adding l2 cache for cpu 0
    if options.l2cache:
        system.cpu[0].l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l2_size,
                                   assoc=options.l2_assoc)
        system.cpu[0].tol2bus = L2XBar(clk_domain = system.cpu_clk_domain)
        system.cpu[0].l2.cpu_side = system.cpu[0].tol2bus.master
        system.cpu[0].l2.mem_side=system.membus.slave
        system.cpu[0].connectAllPorts(system.cpu[0].tol2bus, system.membus)
    elif options.external_memory_system:
        system.cpu[0].connectUncachedPorts(system.membus)
    else:
        system.cpu[0].connectAllPorts(system.membus)
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
