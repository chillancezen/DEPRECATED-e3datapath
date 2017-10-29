#include <e3test.h>
#include <fast-index.h>
#include <rte_malloc.h>

DECLARE_TEST_CASE(tc_fast_index);

START_TEST(findex_2_1_6){
	struct findex_2_1_6_base * base=allocate_findex_2_1_6_base();
	ck_assert_msg(!!base,"OUT OF MEMORY,please preserve to perform test");
	struct findex_2_1_6_key key;
	key.key_index=0x123;
	key.tag0=0;
	key.tag1=0x12;
	key.tag2=0x34;
	key.tag3=0x56;
	key.value_as_u64=0x12345678;
	ck_assert(fast_index_2_1_6_item_safe(base,&key));
	ck_assert(!base[key.key_index].next);
	ck_assert_msg(!add_index_2_1_6_item_unsafe(base,&key),"not enough memory");
	ck_assert(!!base[key.key_index].next);
	key.value_as_u64=0;
	ck_assert(!fast_index_2_1_6_item_safe(base,&key));
	ck_assert(key.value_as_u64==0x12345678);

	key.tag3=0x78;
	key.value_as_u64=0x12345679;
	ck_assert_msg(!add_index_2_1_6_item_unsafe(base,&key),"not enough memory");
	key.value_as_u64=0;
	ck_assert(!fast_index_2_1_6_item_safe(base,&key));
	ck_assert(key.value_as_u64==0x12345679);
	
	key.tag3=0x56;
	delete_index_2_1_6_item_unsafe(base,&key);
	ck_assert(!!base[key.key_index].next);
	key.tag3=0x78;
	delete_index_2_1_6_item_unsafe(base,&key);
	ck_assert(!base[key.key_index].next);
	rte_free(base);
}
END_TEST
ADD_TEST(findex_2_1_6);