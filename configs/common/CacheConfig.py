from __future__ import print_function
from __future__ import absolute_import

import m5
from m5.objects import *
from common.Caches import *
from m5.util import *
from common.Benchmarks import *
from common import ObjectList

def config_cache(options, system):

    opt_mem_type = options.mem_type

    dcache_class, icache_class, l2_cache_class, walk_cache_class = \
        L1_DCache, L1_ICache, L2Cache, None

    walk_cache_class = PageTableWalkerCache
    system.cache_line_size = options.cacheline_size
    
    if opt_mem_type == "HMC_2500_1x32":
        #@PIM - Creating the Cache objects for the PIM CPU
        pim_icache = icache_class(size=options.l1i_size,
                      assoc=options.l1i_assoc)
        pim_dcache = dcache_class(size=options.l1d_size,
                          assoc=options.l1d_assoc)

        #@PIM - Creating PageTableCache for the PIM CPU
        if walk_cache_class:
            pim_iwalkcache = walk_cache_class()
            pim_dwalkcache = walk_cache_class()
        else:
            pim_iwalkcache = None
            pim_dwalkcache = None
    
        #@PIM - Connect all the ports of the created cache object to PIM Cpu
        system.pim_cpu.icache = pim_icache
        system.pim_cpu.dcache = pim_dcache
        system.pim_cpu.icache_port = pim_icache.cpu_side
        system.pim_cpu.dcache_port = pim_dcache.cpu_side
        system.pim_cpu.itb_walker_cache = pim_iwalkcache
        system.pim_cpu.dtb_walker_cache = pim_dwalkcache
        system.pim_cpu.itb.walker.port = pim_iwalkcache.cpu_side
        system.pim_cpu.dtb.walker.port = pim_dwalkcache.cpu_side

        #@PIM - Creating L2 Cache for PIM 
        system.pim_l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                               size=options.l2_size,
                               assoc=options.l2_assoc)

        #@PIM - Creating L2 bus and connecting the L2 Cache Object to it
        system.topim_l2bus = L2XBar(clk_domain = system.cpu_clk_domain)
        system.pim_l2.cpu_side = system.topim_l2bus.master
        system.pim_l2.mem_side = system.pim_bus[0].slave
    
        #@PIM - Connecting the L2 bus to Memory
        system.pim_cpu.icache.mem_side = system.topim_l2bus.slave 
        system.pim_cpu.dcache.mem_side =system.topim_l2bus.slave
        system.pim_cpu.itb_walker_cache.mem_side =system.topim_l2bus.slave
        system.pim_cpu.dtb_walker_cache.mem_side =system.topim_l2bus.slave

    
    #Create Cache for host CPU
    icache = icache_class(size=options.l1i_size,
                      assoc=options.l1i_assoc)
    dcache = dcache_class(size=options.l1d_size,
                          assoc=options.l1d_assoc)

    #Create TableWalkerCache for HOST CPU
    if walk_cache_class:
        iwalkcache = walk_cache_class()
        dwalkcache = walk_cache_class()
    else:
        iwalkcache = None
        dwalkcache = None

    #Connect all the ports of the created cache object to HOST CPU
    system.cpu[0].icache = icache
    system.cpu[0].dcache = dcache
    system.cpu[0].icache_port = icache.cpu_side
    system.cpu[0].dcache_port = dcache.cpu_side
    system.cpu[0].itb_walker_cache = iwalkcache
    system.cpu[0].dtb_walker_cache = dwalkcache
    system.cpu[0].itb.walker.port = iwalkcache.cpu_side
    system.cpu[0].dtb.walker.port = dwalkcache.cpu_side

    #Create threads for the HOST CPU
    system.cpu[0].createInterruptController()
    
    #Create L2 Cache object for the HOST CPU
    system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                           size=options.l2_size,
                           assoc=options.l2_assoc)
    
    #Create a L2 bus and connect the created L2 cache Object to it
    system.tocpu_l2bus = L2XBar(clk_domain = system.cpu_clk_domain)
    system.l2.cpu_side = system.tocpu_l2bus.master
    system.l2.mem_side = system.membus.slave

    #Connect the L2 Bus to memory
    system.cpu[0].icache.mem_side = system.tocpu_l2bus.slave 
    system.cpu[0].dcache.mem_side =system.tocpu_l2bus.slave
    system.cpu[0].itb_walker_cache.mem_side =system.tocpu_l2bus.slave
    system.cpu[0].dtb_walker_cache.mem_side =system.tocpu_l2bus.slave

    #Connect the Created interrupts to the Memory bus
    system.cpu[0].interrupts[0].pio = system.membus.master
    system.cpu[0].interrupts[0].int_master = system.membus.slave
    system.cpu[0].interrupts[0].int_slave = system.membus.master

    return system