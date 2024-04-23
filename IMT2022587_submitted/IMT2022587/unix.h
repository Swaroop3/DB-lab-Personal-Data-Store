#ifndef APPLICATION_H
#define APPLICATION_H

#define UNIVERSITY_SUCCESS 0
#define UNIVERSITY_FAILURE 1
 
struct University{
    int aishe_code;
    char name[50];
    char city[50];
};  

struct Department{
    int dept_id;
    char name[30];
    int personnel;
};

extern struct PDS_RepoInfo *repoHandle;

int add_department(struct Department *department);
int add_university(struct University *university);
int add_university_department(struct University *university, struct Department *department);

int search_university(int aishe_code, struct University *university);
int search_department(int dept_id, struct Department *department);

int search_university_by_city(char *city, struct University *university, int *io_count);

int match_university_city(void *rec, void *key);
int delete_university(int contact_id);
int link_entities(int aishe_code, int dept_id);
int fetch_links(int aishe_code, int linked_keys_result[], int *result_set_size);

#endif
