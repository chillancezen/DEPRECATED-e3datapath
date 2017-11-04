#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <urcu-qsbr.h>
#include <unistd.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <node.h>
#include <node_class.h>
#include <node_adjacency.h>
#include <lcore_extension.h>
#include <e3_init.h>
#include <e3interface.h>
//#include <spine-label-fib.h>
//#include <spine-label-nhlfe.h>
int
main(int argc, char **argv)
{
	
	int ret;
	unsigned lcore_id;
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");
	init_registered_tasks();
	//label_fib_module_test();
	//label_nhlfe_module_test();
	//dump_e3_interface_structure();
	dump_e3_interface_structure();
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		rte_eal_remote_launch(lcore_default_entry, NULL, lcore_id);
	}
	lcore_default_entry(NULL);
	rte_eal_mp_wait_lcore();
	return 0;
}
