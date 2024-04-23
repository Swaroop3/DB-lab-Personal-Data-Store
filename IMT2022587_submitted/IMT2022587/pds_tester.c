#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "unix.h"



#define TREPORT(a1,a2) printf("Status: %s - %s\n\n",a1,a2); fflush(stdout);

void process_line( char *test_case );

int main(int argc, char *argv[])
{
	FILE *cfptr;
	char test_case[50];

	if( argc != 2 ){
		fprintf(stderr, "Usage: %s testcasefile\n", argv[0]);
		exit(1);
	}

	cfptr = (FILE *) fopen(argv[1], "r");
	while(fgets(test_case, sizeof(test_case)-1, cfptr)){
		// printf("line:%s",test_case);
		if( !strcmp(test_case,"\n") || !strcmp(test_case,"") )
			continue;
		process_line( test_case );
	}
}

void process_line( char *test_case )
{
	char university_repo_name[30], department_repo_name[30];
	char command[15], param1[20], param2[20], param3[20],info[1024];
	int aishe_code, dept_id, status, university_rec_size, department_rec_size, expected_status;
	struct University university;struct Department department;

	// strcpy(testContact.contact_name, "dummy name");
	// strcpy(testContact.phone, "dummy number");

	university_rec_size = sizeof(struct University);	
	department_rec_size = sizeof(struct Department);

	sscanf(test_case, "%s%s%s%s", command, param1, param2, param3);
	printf("Test case: %s", test_case); fflush(stdout);
	if( !strcmp(command,"CREATE") ){
		strcpy(university_repo_name, param1);
		strcpy(department_repo_name, param2);
		if( !strcmp(param3,"0") )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;

		status = pds_create( university_repo_name,department_repo_name );
		if(status == PDS_SUCCESS)
			status = UNIVERSITY_SUCCESS;
		else
			status = UNIVERSITY_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"pds_create returned status %d",status);
			TREPORT("FAIL", info);
			exit(0);
		}
	}
	else if( !strcmp(command,"OPEN") ){
		strcpy(university_repo_name, param1);
		strcpy(department_repo_name, param2);
		if( !strcmp(param3,"0") )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;

		status = pds_open(university_repo_name, department_repo_name, university_rec_size, department_rec_size);
		if(status == PDS_SUCCESS)
			status = UNIVERSITY_SUCCESS;
		else
			status = UNIVERSITY_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"pds_open returned status %d",status);
			TREPORT("FAIL", info);
			exit(0);
		}
	}
	else if( !strcmp(command,"STORE") ){
		if( !strcmp(param3,"0") )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;

		if(strcmp(param1,"NULL")!=0 && strcmp(param2,"NULL")==0){
			sscanf(param1, "%d", &aishe_code);
			university.aishe_code = aishe_code;
			sprintf(university.city, "City-of-%d", aishe_code);
			sprintf(university.name, "Name-of-%d", aishe_code);
			status = add_university( &university );
			if(status == PDS_SUCCESS)
				status = UNIVERSITY_SUCCESS;
			else
				status = UNIVERSITY_FAILURE;
			if( status == expected_status ){
				TREPORT("PASS", "");
			}
			else{
				sprintf(info,"add_parent returned status %d",status);
				TREPORT("FAIL", info);
				exit(0);
			}
		}
		else if(strcmp(param1,"NULL")==0 && strcmp(param2,"NULL")!=0){
			sscanf(param2, "%d", &dept_id);
			department.dept_id = dept_id;
			department.personnel = dept_id * 100;
			sprintf(department.name, "Name-of-%d", dept_id);
			status = add_department( &department );
			if(status == PDS_SUCCESS)
				status = UNIVERSITY_SUCCESS;
			else
				status = UNIVERSITY_FAILURE;
			if( status == expected_status ){
				TREPORT("PASS", "");
			}
			else{
				sprintf(info,"add_child returned status %d",status);
				TREPORT("FAIL", info);
				exit(0);
			}
		}
		else{
			sscanf(param2, "%d", &dept_id);
			sscanf(param1, "%d", &aishe_code);
			department.dept_id = dept_id;
			department.personnel = dept_id * 100;
			sprintf(department.name, "Name-of-%d", dept_id);
			university.aishe_code = aishe_code;
			sprintf(university.city, "City-of-%d", aishe_code);
			sprintf(university.name, "Name-of-%d", aishe_code);
			status = add_university_department( &university, &department);
			if(status == PDS_SUCCESS)
				status = UNIVERSITY_SUCCESS;
			else
				status = UNIVERSITY_FAILURE;
			if( status == expected_status ){
				TREPORT("PASS", "");
			}
			else{
				sprintf(info,"add_parent_child returned status %d",status);
				TREPORT("FAIL", info);
			}
		}
	}
	else if( !strcmp(command,"NDX_SEARCH") ){
		if( !strcmp(param3,"0") )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;
		if(strcmp(param1,"NULL")!=0 && strcmp(param2,"NULL")==0){
			sscanf(param1, "%d", &aishe_code);
			university.aishe_code = -1;
			status = search_university( aishe_code, &university );
			if(status == PDS_SUCCESS)
				status = UNIVERSITY_SUCCESS;
			else
				status = UNIVERSITY_FAILURE;
			if( status != expected_status ){
				sprintf(info,"search key: %d; Got status %d",aishe_code, status);
				TREPORT("FAIL", info);
				exit(0);
			}
			else{
				// Check if the retrieved values match
				char expected_name[30];
				sprintf(expected_name, "Name-of-%d", aishe_code);
				char expected_city[30];
				sprintf(expected_city, "City-of-%d", aishe_code);
				if( expected_status == 0 ){
					if (university.aishe_code == aishe_code && 
						strcmp(university.name,expected_name) == 0 &&
						strcmp(university.city, expected_city) == 0){
							TREPORT("PASS", "");
					}
					else{
						sprintf(info,"University data not matching... Expected:{%d,%s,%s} Got:{%d,%s,%s}\n",
							aishe_code, expected_name, expected_city, 
							university.aishe_code, university.name, university.city
						);
						TREPORT("FAIL", info);
						exit(0);
					}
				}
				else
					TREPORT("PASS", "");
			}
		}
		else if(strcmp(param1,"NULL")==0 && strcmp(param2,"NULL")!=0){
			sscanf(param2, "%d", &dept_id);
			department.dept_id = -1;
			status = search_department( dept_id, &department );
			if(status == PDS_SUCCESS)
				status = UNIVERSITY_SUCCESS;
			else
				status = UNIVERSITY_FAILURE;
			if( status != expected_status ){
				sprintf(info,"search key: %d; Got status %d",dept_id, status);
				TREPORT("FAIL", info);
				exit(0);
			}
			else{
				// Check if the retrieved values match
				char expected_name[30];
				sprintf(expected_name, "Name-of-%d", dept_id);
				if( expected_status == 0 ){
					if (department.dept_id == dept_id && 
						strcmp(department.name,expected_name) == 0 &&
						department.personnel == dept_id * 100){
							TREPORT("PASS", "");
						}
					else{
						sprintf(info,"Department data not matching... Expected:{%d,%s,%d} Got:{%d,%s,%d}\n",
							dept_id, expected_name, dept_id*100, 
							department.dept_id, department.name, department.personnel
						);
						TREPORT("FAIL", info);
						exit(0);
					}
				}
				else
					TREPORT("PASS", "");
			}
		}
	}
	else if( !strcmp(command,"NON_NDX_SEARCH") ){
		char city[30], expected_name[30], expected_city[30];
		int expected_io, actual_io;
		
		if( strcmp(param3,"-1") == 0 )
			expected_status = UNIVERSITY_FAILURE;
		else
			expected_status = UNIVERSITY_SUCCESS;

		sscanf(param1, "%s", city);
		sscanf(param3, "%d", &expected_io);
		university.aishe_code = -1;
		actual_io = 0;
		status = search_university_by_city( city, &university, &actual_io );
		if(status == PDS_SUCCESS)
			status = UNIVERSITY_SUCCESS;
		else
			status = UNIVERSITY_FAILURE;
		if( status != expected_status ){
			sprintf(info,"search key: %d; Got status %d",aishe_code, status);
			TREPORT("FAIL", info);
			exit(0);
		}
		else{
			// Check if the retrieved values match
			// Check if num block accesses match too
			// Extract the expected contact_id from the phone number
			sscanf(city+sizeof("City-of"), "%d", &aishe_code);
			sprintf(expected_name,"Name-of-%d",aishe_code);
			sprintf(expected_city,"City-of-%d",aishe_code);
			if( expected_status == 0 ){
				if (university.aishe_code == aishe_code && 
					strcmp(university.name, expected_name) == 0 &&
					strcmp(university.city, expected_city) == 0 ){
						if( expected_io == actual_io ){
							TREPORT("PASS", "");
						}
						else{
							sprintf(info,"Num I/O not matching for university %d... Expected:%d Got:%d\n",
								aishe_code, expected_io, actual_io
							);
							TREPORT("FAIL", info);
							exit(0);
						}
				}
				else{
					sprintf(info,"University data not matching... Expected:{%d,%s,%s} Got:{%d,%s,%s}\n",
						aishe_code, expected_name, expected_city, 
						university.aishe_code, university.name, university.city
					);
					TREPORT("FAIL", info);
					exit(0);
				}
			}
			else
				TREPORT("PASS", "");
		}
	}
	else if( !strcmp(command,"NDX_DELETE") ){
		if( strcmp(param3,"0") == 0 )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;

		sscanf(param1, "%d", &aishe_code);
		university.aishe_code = -1;
		status = delete_university( aishe_code );
		if( status != expected_status ){
			sprintf(info,"delete key: %d; Got status %d",aishe_code, status);
			TREPORT("FAIL", info);
			exit(0);
		}
		else{
			TREPORT("PASS", "");
		}
	}
	else if( !strcmp(command,"CLOSE") ){
		if( !strcmp(param1,"0") )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;

		status = pds_close();
		if(status == PDS_SUCCESS)
			status = UNIVERSITY_SUCCESS;
		else
			status = UNIVERSITY_FAILURE;
		if( status == expected_status ){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"pds_close returned status %d",status);
			TREPORT("FAIL", info);
			exit(0);
		}
	}
	else if(!strcmp(command,"LINK")){
		if( !strcmp(param3,"0") )
			expected_status = UNIVERSITY_SUCCESS;
		else
			expected_status = UNIVERSITY_FAILURE;
		sscanf(param1, "%d", &aishe_code);
		sscanf(param2, "%d", &dept_id);
		status = link_entities(aishe_code,dept_id);
		if(status == expected_status){
			TREPORT("PASS", "");
		}
		else{
			sprintf(info,"link_entities returned status %d",status);
			TREPORT("FAIL", info);
			exit(0);
		}
	}
	else if(!strcmp(command,"LINK_SEARCH")){
		int expected_set_size;
		if( strcmp(param3,"-1")==0 )
			expected_status = UNIVERSITY_FAILURE;
		else
			expected_status = UNIVERSITY_SUCCESS;
		sscanf(param1, "%d", &aishe_code);
		sscanf(param3, "%d", &expected_set_size);
		int linked_keys_result[10];
		int result_set_size;
		status = fetch_links(aishe_code,linked_keys_result,&result_set_size);
		if(status==expected_status){
			if(status==UNIVERSITY_SUCCESS){
				if(result_set_size==expected_set_size){
					TREPORT("PASS", "");
				}
				else{
					sprintf(info,"fetch_links returned result set size %d\n",result_set_size);
					TREPORT("FAIL", info);
					exit(0);
				}
			}
			else{
				TREPORT("PASS", "");
			}
		}
		else{
			sprintf(info,"fetch_links returned status %d\n",status);
			TREPORT("FAIL", info);
			exit(0);
		}
	}
	else{
		TREPORT("FAIL", "Invalid command");
	}
}


