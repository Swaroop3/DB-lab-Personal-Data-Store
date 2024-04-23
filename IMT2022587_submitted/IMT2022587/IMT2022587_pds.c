#include<stdio.h>
#include "pds.h"
#include "bst.h"
#include<string.h>
#include<stdlib.h>
#include"unix.h"

struct PDS_RepoInfo repo_handle;

int pds_create(char *repo_name, char *linked_repo_name)
{
	char *temp_dat = (char *)malloc(sizeof(repo_name));
	char *temp_ndx = (char *)malloc(sizeof(repo_name));
	char *temp_linked_dat = (char *)malloc(sizeof(linked_repo_name));
	char *temp_link_dat = (char *)malloc(sizeof(repo_name)+sizeof(linked_repo_name));

	strcpy(temp_dat, repo_name);
	strcpy(temp_ndx, repo_name);
	strcpy(temp_linked_dat, linked_repo_name);
	strcpy(temp_link_dat, repo_name);strcat(temp_link_dat, "_");strcat(temp_link_dat, linked_repo_name);

	strcat(temp_dat, ".dat");
	strcat(temp_ndx, ".ndx");
	strcat(temp_linked_dat, ".dat");
	strcat(temp_link_dat, ".dat");
	
	repo_handle.pds_data_fp = fopen(temp_dat, "wb");
	repo_handle.pds_ndx_fp = fopen(temp_ndx, "wb");
	
	int flag = (linked_repo_name == NULL) ? 0 :1;
	
	if (flag != 0)
	{
		repo_handle.pds_linked_data_fp = fopen(temp_linked_dat, "wb");
		repo_handle.pds_link_fp = fopen(temp_link_dat , "wb");
	}
	
	free(temp_dat);
	free(temp_ndx);
	free(temp_linked_dat);
	free(temp_link_dat);

	if(!repo_handle.pds_data_fp || !repo_handle.pds_ndx_fp || ((flag !=0 ) && (!repo_handle.pds_linked_data_fp || !repo_handle.pds_link_fp)))
		return PDS_FILE_ERROR;

	int zero = 0;
	fwrite(&zero, sizeof(int), 1, repo_handle.pds_ndx_fp);
	fclose(repo_handle.pds_data_fp);
	fclose(repo_handle.pds_ndx_fp);
	if (flag != 0)
	{
		fclose(repo_handle.pds_linked_data_fp);
		fclose(repo_handle.pds_link_fp);
	}
	repo_handle.repo_status = PDS_REPO_CLOSED;
	return PDS_SUCCESS;
}

int pds_open( char *repo_name, char *linked_repo_name, int rec_size, int linked_rec_size )
{
	if(repo_handle.repo_status==PDS_REPO_OPEN)
		return PDS_REPO_ALREADY_OPEN;
	
	FILE *f1 = NULL;
	FILE *f2 = NULL;
	FILE *f3 = NULL;
	FILE *f4 = NULL;

	char *temp_dat[30];
	char *temp_ndx[30];
	char *temp_linked_dat[30];
	char *temp_link_dat[30];

	strcpy(temp_dat, repo_name);
	strcpy(temp_ndx, repo_name);
	strcpy(temp_linked_dat, linked_repo_name);
	strcpy(temp_link_dat, repo_name);strcat(temp_link_dat, "_");strcat(temp_link_dat, linked_repo_name);

	strcat(temp_dat, ".dat");
	strcat(temp_ndx, ".ndx");
	strcat(temp_linked_dat, ".dat");
	strcat(temp_link_dat, ".dat");
	
	f1 = fopen(temp_dat, "rb+");
	f2 = fopen(temp_ndx, "rb+");
	
	int flag = (linked_repo_name == NULL) ? 0 :1;
	
	if (flag != 0)
	{
		f3 = fopen(temp_linked_dat, "rb+");
		f4 = fopen(temp_link_dat , "rb+");
	}
	else
	{
		f3 = NULL;
		f4 = NULL;
	}
	
	if(!repo_handle.pds_data_fp || !repo_handle.pds_ndx_fp || ((flag !=0 ) && (!repo_handle.pds_linked_data_fp || !repo_handle.pds_link_fp)))
		return PDS_FILE_ERROR;
	
	printf("haha\n");
	
	strcpy(repo_handle.pds_name,repo_name);
	repo_handle.pds_data_fp = f1;
	repo_handle.pds_ndx_fp = f2;
	repo_handle.pds_linked_data_fp = f3;
	repo_handle.pds_link_fp = f4;
	repo_handle.repo_status = PDS_REPO_OPEN;
	repo_handle.rec_size = rec_size;
	repo_handle.linked_rec_size = linked_rec_size;
	repo_handle.pds_bst = NULL;
	fread(&repo_handle.rec_count, sizeof(int), 1, repo_handle.pds_ndx_fp);
	if( pds_load_ndx() == PDS_FILE_ERROR )
		return PDS_FILE_ERROR;
	fclose(f2);
	return PDS_SUCCESS;
}

int pds_load_ndx()
{
	for(int i=0;i<repo_handle.rec_count;i++)
	{
		struct PDS_NdxInfo* node = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
		if(repo_handle.repo_status == PDS_REPO_CLOSED)
			return PDS_REPO_NOT_OPEN;
		if(fread(node,sizeof(struct PDS_NdxInfo),1,repo_handle.pds_ndx_fp) != 1)
			return PDS_FILE_ERROR;
		int bst_status = bst_add_node(&repo_handle.pds_bst,node->key,node);
		if(bst_status == BST_DUP_KEY || bst_status == BST_NULL)
			return PDS_NDX_SAVE_FAILED;
	}
	return PDS_SUCCESS;
}


int put_rec_by_key( int key, void *rec )
{
    if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
	fseek(repo_handle.pds_data_fp, 0, SEEK_END);
	struct PDS_NdxInfo* ndx = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
	int offset = ftell(repo_handle.pds_data_fp);
	ndx->key = key;
	ndx->offset = offset;
	ndx->is_deleted = 0;
	struct BST_Node *node = NULL;
	node = bst_search(repo_handle.pds_bst,key);
	if(node != NULL)
		if(((struct PDS_NdxInfo*)node->data)->is_deleted)
		{
			((struct PDS_NdxInfo*)node->data)->is_deleted=0;
			return PDS_SUCCESS;
		}
	int bst_status = bst_add_node(&repo_handle.pds_bst,key,ndx);
	if(bst_status != BST_SUCCESS)
		return PDS_ADD_FAILED;
	repo_handle.rec_count++;
	fwrite(&key,sizeof(int),1,repo_handle.pds_data_fp);
	fwrite(rec,repo_handle.rec_size,1,repo_handle.pds_data_fp);
    return PDS_SUCCESS;
}


int put_linked_rec_by_key( int key, void *rec )
{
    if(repo_handle.pds_linked_data_fp == NULL)
		return PDS_FILE_ERROR;
	if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
    fseek(repo_handle.pds_linked_data_fp, 0, SEEK_END);
    fwrite(&key, sizeof(int), 1, repo_handle.pds_linked_data_fp);
    fwrite(rec, repo_handle.linked_rec_size, 1, repo_handle.pds_linked_data_fp);
    return PDS_SUCCESS;
}


int get_rec_by_ndx_key( int key, void *rec )
{
	if(repo_handle.repo_status != PDS_REPO_OPEN)
    	return PDS_REPO_NOT_OPEN;
	struct BST_Node *node = (struct BST_Node*)malloc(sizeof(struct BST_Node));
	if((node = bst_search(repo_handle.pds_bst,key)) == NULL)
		return PDS_REC_NOT_FOUND;
	struct PDS_NdxInfo *ndx = (struct PDS_NdxInfo*)node->data;
	if(ndx->is_deleted == 1)
		return PDS_REC_NOT_FOUND;
	fseek(repo_handle.pds_data_fp, ndx->offset, SEEK_SET);
	fseek(repo_handle.pds_data_fp, sizeof(int), SEEK_CUR);
	fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
	return PDS_SUCCESS;
}

int get_rec_by_non_ndx_key(void *non_ndx_key, void *rec, int (*matcher)(void *rec, void *non_ndx_key), int *io_count)
{
	if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
	fseek(repo_handle.pds_data_fp, 0, SEEK_SET);
	int *key = malloc(sizeof(int));
	while(!feof(repo_handle.pds_data_fp))
	{
		fread(key, sizeof(int), 1, repo_handle.pds_data_fp);
		fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
		(*io_count)++;
		struct BST_Node* node = (struct BST_Node*)bst_search(repo_handle.pds_bst,*key);
		struct PDS_NdxInfo* ndx = (struct PDS_NdxInfo*)node->data;
		if(matcher(rec,non_ndx_key) == 0 && ndx->is_deleted == 0)
			return PDS_SUCCESS;
	}
	return PDS_REC_NOT_FOUND;
}

int get_linked_rec_by_key( int key, void *rec )
{
    if(repo_handle.pds_linked_data_fp == NULL)
		return PDS_FILE_ERROR;
	if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
    fseek(repo_handle.pds_linked_data_fp, 0, SEEK_SET);
    int* keyit = malloc(sizeof(int));
    while(fread(keyit,sizeof(int),1,repo_handle.pds_linked_data_fp))
	{
		fread(rec, repo_handle.linked_rec_size, 1, repo_handle.pds_linked_data_fp);
		if(*keyit==key)
			return PDS_SUCCESS;
    }
	return PDS_REC_NOT_FOUND;
}

int delete_rec_by_ndx_key( int key )
{
	struct BST_Node * node;
	if ((node = bst_search(repo_handle.pds_bst, key)) == NULL)
		return PDS_DELETE_FAILED;
	struct PDS_NdxInfo *ndx = (struct PDS_NdxInfo *)(node->data);
	if (ndx->is_deleted)
		return PDS_DELETE_FAILED;
	ndx->is_deleted = 1;
	return PDS_SUCCESS;
}

int pds_link_rec (int key1, int key2)
{
	if(repo_handle.pds_linked_data_fp == NULL)
		return PDS_FILE_ERROR;
	if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
	struct PDS_link_info* link = (struct PDS_link_info*)malloc(sizeof(struct PDS_link_info));
	link->parent_key = key1;
	link->child_key = key2;
	fseek(repo_handle.pds_link_fp, 0, SEEK_END);
	fwrite(link, sizeof(struct PDS_link_info), 1, repo_handle.pds_link_fp);
	return PDS_SUCCESS;
}

int pds_get_linked_rec(int parent_key, int* linked_keys_result,  int* result_set_size){
	if(repo_handle.pds_linked_data_fp == NULL)
		return PDS_FILE_ERROR;
	if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
	fseek(repo_handle.pds_link_fp, 0, SEEK_SET);
	*result_set_size = 0;
	struct PDS_link_info *link = (struct PDS_link_info*)malloc(sizeof(struct PDS_link_info));
	while(fread(link,sizeof(struct PDS_link_info),1,repo_handle.pds_link_fp))
		if(link->parent_key == parent_key)
		{
			linked_keys_result[*result_set_size]=link->child_key;
			(*result_set_size)++;
		}
}

int bst_pre_order(struct BST_Node *node){
	if(!node)
		return 0;
	struct PDS_NdxInfo* ndx =(struct PDS_NdxInfo*)node->data;
	fwrite(node->data,sizeof(struct PDS_NdxInfo),1,repo_handle.pds_ndx_fp);
	bst_pre_order(node->left_child);
	bst_pre_order(node->right_child);
}

int pds_close(){
	if(repo_handle.repo_status == PDS_REPO_CLOSED)
		return PDS_REPO_NOT_OPEN;
	
	char *temp = (char *)malloc(sizeof(repo_handle.pds_name));
	strcpy(temp, repo_handle.pds_name);
	strcat(temp, ".ndx");
	repo_handle.pds_ndx_fp = fopen(temp, "wb");
	free(temp);
	fwrite(&repo_handle.rec_count, sizeof(int), 1, repo_handle.pds_ndx_fp);
	// bst_print_custom(repo_handle.pds_bst);
	bst_pre_order(repo_handle.pds_bst);
	bst_destroy(repo_handle.pds_bst);
	fclose(repo_handle.pds_data_fp);
	fclose(repo_handle.pds_ndx_fp);
	if(repo_handle.pds_linked_data_fp)
		fclose(repo_handle.pds_linked_data_fp);
	repo_handle.repo_status = PDS_REPO_CLOSED;
	return PDS_SUCCESS;
}
