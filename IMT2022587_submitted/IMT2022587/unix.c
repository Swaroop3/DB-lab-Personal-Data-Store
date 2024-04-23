#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "unix.h"

int search_university( int aishe_code, struct University *university )
{
	return get_rec_by_ndx_key( aishe_code, university );
}

int search_department( int dept_id, struct Department *department )
{
	return get_linked_rec_by_key( dept_id, department );
}

int add_university( struct University *university )
{
	int status = put_rec_by_key( university->aishe_code, university );
	if( status == PDS_SUCCESS )
		return status;
	fprintf(stderr, "add university fail, key: %d, error %d\n", university->aishe_code, status );
	return UNIVERSITY_FAILURE;
}

int add_department(struct Department *department)
{
	int status = put_linked_rec_by_key( department->dept_id, department );
	if( status == PDS_SUCCESS )
		return status;
	fprintf(stderr, "add department fail, key %d, error %d", department->dept_id, status );
	return UNIVERSITY_FAILURE;
}

int add_university_department(struct University *university, struct Department *department){
	int status1, status2;
	status1 = add_university(university);
	status2 = add_department(department);
	if(status1==UNIVERSITY_SUCCESS && status2==UNIVERSITY_SUCCESS) return UNIVERSITY_SUCCESS;
	else return UNIVERSITY_FAILURE;
}

int search_university_by_city( char *city, struct University *university, int *io_count )
{
	if(get_rec_by_non_ndx_key(city, university, match_university_city, io_count))
		return UNIVERSITY_FAILURE;
	return UNIVERSITY_SUCCESS;
}

int match_university_city( void *rec, void *key )
{
	struct University* university = (struct University*)malloc(sizeof(struct University));
	university = rec;
	char* k = key;
	if(strcmp(university->city, k) == 0)
		return 0;
	return 1;
}

int delete_university(int aishe_code)
{
	if(delete_rec_by_ndx_key(aishe_code) == 0)
		return 0;
	return 1;
}

int link_entities(int aishe_code, int dept_id)
{
	struct University* university = (struct University*)malloc(sizeof(struct University));
	struct Department* department = (struct Department*)malloc(sizeof(struct Department));
	if(get_linked_rec_by_key(dept_id, department) == 0 && get_rec_by_ndx_key(aishe_code, university) == 0){
		int test[10];
		int *test_size = (int*)malloc(sizeof(int));
		pds_get_linked_rec(aishe_code, test, test_size);
		for(int i=0; i<*test_size; i++){
			if(test[i] == dept_id) return UNIVERSITY_FAILURE;
		}
		if(pds_link_rec(aishe_code, dept_id) == PDS_SUCCESS)
			return UNIVERSITY_SUCCESS;
		return UNIVERSITY_FAILURE;
	}
	return UNIVERSITY_FAILURE;
}

int fetch_links(int aishe_code, int linked_keys_result[], int *result_set_size)
{
	struct University* university = (struct University*)malloc(sizeof(struct University));
	if(get_rec_by_ndx_key(aishe_code, university) == PDS_SUCCESS){
		if(pds_get_linked_rec(aishe_code, linked_keys_result, result_set_size) == PDS_SUCCESS)
			return UNIVERSITY_SUCCESS;
		return UNIVERSITY_FAILURE;
	}
	return UNIVERSITY_FAILURE;
}
