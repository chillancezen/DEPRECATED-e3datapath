#include <e3test.h>
#include <e3net/include/common-nhlfe.h>
#include <util.h>
DECLARE_TEST_CASE(tc_common_nhlfe);

START_TEST(common_nhlfe_general){
	int idx=0;
	int last_offset=0,last_size=0;
	printf("dump definition of common_neighbor:\n");
	dump_field(struct common_neighbor,neighbour_ip_as_le);
	dump_field(struct common_neighbor,ref_cnt);
	dump_field(struct common_neighbor,index);
	dump_field(struct common_neighbor,mac);
	dump_field(struct common_neighbor,is_valid);
	dump_field(struct common_neighbor,reserved0);

	struct common_neighbor neighbor;
	neighbor.neighbour_ip_as_le=0x1;
	
	/*
	*maximmum number of neighbors supported
	*/
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++){
		neighbor.neighbour_ip_as_le++;
		ck_assert(register_common_neighbor(&neighbor)>=0);
	}
	neighbor.neighbour_ip_as_le++;
	ck_assert(register_common_neighbor(&neighbor)<0);
	
}
END_TEST
ADD_TEST(common_nhlfe_general);