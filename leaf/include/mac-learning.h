#ifndef _MAC_LEARNING_H
#define _MAC_LEARNING_H
#include <inttypes.h>
#include <e3infra/include/fast-index.h>
#include <e3infra/include/node.h>
#include <e3infra/include/node-class.h>
#include <rte_ring.h>
#include <e3infra/include/util.h>
#include <leaf/include/leaf-e-service.h>
#define MAC_LEARNING_CLASSNAME "mac-learn-class"
/*
*use a cache array
*/
struct mac_learning_cache_entry{
	union{
		void * u64[2];
		struct{
			uint8_t mac[6];
			uint16_t elan_index;
			uint64_t fwd_entry_as64; 
		};
	};
}__attribute__((packed));
extern struct node_class * pmac_learning_class;

__attribute__((always_inline))
	inline static int update_mac_learning_cache(struct mac_learning_cache_entry * cache_entry,uint64_t ts_now)
{
	int idx;
	struct findex_2_4_key key;
	struct ether_e_lan * elan;
	struct node * pnode;
	mac_to_findex_2_4_key(cache_entry->mac,&key);
	key.leaf_fwd_entry_as_64=cache_entry->fwd_entry_as64;
	key.ts_last_updated=ts_now;
	
	elan=find_e_lan_service(cache_entry->elan_index);
	if(PREDICT_FALSE(!elan))
		return -E3_ERR_ILLEGAL;
	if(PREDICT_TRUE(!update_index_2_4_item_safe(elan->fib_base,&key)))
		return E3_OK;
	
	/*
	*send them to MAC_LEARNNG node class.
	*with bulk enqueuing API.
	*/
	for(idx=0;idx<MAX_NODE_IN_CLASS;idx++){
		if(!pmac_learning_class->node_entries[idx].is_valid)
			continue;
		if(!(pnode=find_node_by_index(pmac_learning_class->node_entries[idx].node_index)))
			continue;
		if(2==rte_ring_mp_enqueue_bulk(pnode->node_ring,cache_entry->u64,2,NULL))
			return E3_OK;
	}
	/*
	*no mac-learning node can receive mac_learning entry
	*/
	return -E3_ERR_OUT_OF_RES;
}
#endif