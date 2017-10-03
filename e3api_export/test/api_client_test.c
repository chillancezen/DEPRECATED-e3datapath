#include <e3-api-wrapper.h>
#include <exported-api.h>
#include <assert.h>
#include <stdio.h>
int main()
{
	uint64_t version=0,rc;
	struct e3_api_client * client;
	client=allocate_e3_api_client("tcp://localhost:507");
	assert(client);
	publish_e3_api_client(client);
	
	rc=e3datapath_version(&version);
	printf("%lld,%llx\n",rc,version);
	return 0;
}
