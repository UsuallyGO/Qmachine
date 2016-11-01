
#include <stdio.h>
#include "qmachine/qmachine.h"

int main()
{
	int res;
	Qmachine qm;

	qm  = qm_create();
	res = qm_assemblyTobin(qm, "qm.ass");
	qm_instrDebug(qm);
	qm_run(qm);

	//MemoryCheck();
	return 0;
}
