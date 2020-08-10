

#In this Cache Configuartion file PIM is directly connected to membus where as host is connnected[0] 



from __future__ import print_function
from __future__ import absolute_import

import m5
from m5.objects import *
from common.Caches import *
from m5.util import *
from common.Benchmarks import *
from common import ObjectList

def config_cache(options, system):

    dcache_class, icache_class, l2_cache_class, walk_cache_class = \
        L1_DCache, L1_ICache, L2Cache, None

    walk_cache_class = PageTableWalkerCache
    system.cache_line_size = options.cacheline_size
    
    # CONNECT CACHES TO PIM CPU
    
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
    
    system.pim_cpu.icache = icache
    system.pim_cpu.dcache = dcache
    system.pim_cpu.icache_port = icache.cpu_side
    system.pim_cpu.dcache_port = dcache.cpu_side
    system.pim_cpu.itb_walker_cache = iwalkcache
    system.pim_cpu.dtb_walker_cache = dwalkcache
    system.pim_cpu.itb.walker.port = iwalkcache.cpu_side
    system.pim_cpu.dtb.walker.port = dwalkcache.cpu_side
    
    system.pim_l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                           size=options.l2_size,
                           assoc=options.l2_assoc)
    
    system.topim_l2bus = L2XBar(clk_domain = system.cpu_clk_domain)
    
    system.pim_l2.cpu_side = system.topim_l2bus.master
    system.pim_l2.mem_side = system.pim_bus[0].slave
    
    system.pim_cpu.icache.mem_side = system.topim_l2bus.slave 
    system.pim_cpu.dcache.mem_side =system.topim_l2bus.slave
    system.pim_cpu.itb_walker_cache.mem_side =system.topim_l2bus.slave
    system.pim_cpu.dtb_walker_cache.mem_side =system.topim_l2bus.slave

    # create another bus which will be conneted to the system.membus and host
    
    #system.host_bus = SystemXBar()

    #CONNECT CACHE TO HOST CPU
    
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
    
    system.cpu[0].icache = icache
    system.cpu[0].dcache = dcache
    system.cpu[0].icache_port = icache.cpu_side
    system.cpu[0].dcache_port = dcache.cpu_side
    system.cpu[0].itb_walker_cache = iwalkcache
    system.cpu[0].dtb_walker_cache = dwalkcache
    system.cpu[0].itb.walker.port = iwalkcache.cpu_side
    system.cpu[0].dtb.walker.port = dwalkcache.cpu_side

    system.cpu[0].createInterruptController()
    
    system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                           size=options.l2_size,
                           assoc=options.l2_assoc)
    
    system.tocpu_l2bus = L2XBar(clk_domain = system.cpu_clk_domain)

    system.l2.cpu_side = system.tocpu_l2bus.master
    system.l2.mem_side = system.membus.slave

    system.cpu[0].icache.mem_side = system.tocpu_l2bus.slave 
    system.cpu[0].dcache.mem_side =system.tocpu_l2bus.slave
    system.cpu[0].itb_walker_cache.mem_side =system.tocpu_l2bus.slave
    system.cpu[0].dtb_walker_cache.mem_side =system.tocpu_l2bus.slave

    system.cpu[0].interrupts[0].pio = system.membus.master
    system.cpu[0].interrupts[0].int_master = system.membus.slave
    system.cpu[0].interrupts[0].int_slave = system.membus.master

    return system