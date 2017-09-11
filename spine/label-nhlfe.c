#include <label-nhlfe.h>
#include <e3_init.h>
#include <e3_log.h>
#include <rte_malloc.h>

struct topological_neighbor * topological_neighbor_base;
struct next_hop             * next_hop_base;

void label_nhlfe_init(void)
{
	topological_neighbor_base=rte_zmalloc(NULL,
			sizeof(struct topological_neighbor)*MAX_TOPOLOGICAL_NEIGHBOURS,
			64);
	E3_ASSERT(topological_neighbor_base);
	next_hop_base=rte_zmalloc(NULL,
			sizeof(struct next_hop)*MAX_NEXT_HOPS,
			64);
	E3_ASSERT(next_hop_base);
	
}
E3_init(label_nhlfe_init, TASK_PRIORITY_RESOURCE_INIT);

void label_nhlfe_module_test(void)
{
	printf("%d\n",sizeof(struct topological_neighbor));
	printf("%d\n",sizeof(struct next_hop));
	
}