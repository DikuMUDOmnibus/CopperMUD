#include <stdio.h>

main()
{
	FILE *fl;
	int ver=3,rev=-1;

	if(!(fl=fopen("version.c","r+"))) {
		fprintf(stderr,"Cannot open version.c!\n");
		exit(1);
	}
	fscanf(fl,"char *vers=\"%d.%d\";",&ver,&rev);

	rev++;

	rewind(fl);

	fprintf(fl,"char *vers=\"%d.%d\";\n\n\n\n",ver,rev);

	fclose(fl);

	printf("Bumped version to %d.%d\n",ver,rev);

	return 0;
}
