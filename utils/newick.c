#include "newick.h"
#include <sys/types.h>
#include <regex.h>
#include <string.h>

void
from_edges_to_n_tree(pnt n_tree, tte *edges, int *map, 
			int num_input_genome, int num_edge);
void
free_n_tree(pnt n_tree);
void
from_n_tree_to_newick(pnt n_tree, FILE *writer);
int
find_pattern(char *nw_string, char *pattern,
		int *start, int *end);
void
replace_nwstring_by_int(char *nw_string, int start,
		int end, int len, 
		int val);
int
nw_string_to_int(char *nw_string, int start, 
		int end);
void
from_nwstring_to_three_edges(pt tree, char *nw_string,
		int start, int end, int len);
void
from_nwstring_to_two_edge(pt tree, char *nw_string,
		int start, int end, int len);
int 
readline(FILE *f, char *buffer, 
		size_t len);
int
find_pattern(char *nw_string, char *pattern,
		int *start, int *end);


/*
 * this is used to transfer trees that generated from spectral partition method 
 * into the newick formated tree.
 **/
void 
from_edge_to_newick(FILE *writer, pt trees, 
			int *map){
	int i, j, k;
	pnt n_tree = (pnt)malloc(sizeof(tnt));
	from_edges_to_n_tree(n_tree, trees->edges, map, 
			trees->orders->num_input_genome, 
			trees->num_edge);
	from_n_tree_to_newick(n_tree, writer);
	free_n_tree(n_tree);
}

/*
 *this is for transfer from pnt formatted tree into the newick format 
 **/
void
from_n_tree_to_newick(pnt n_tree, FILE *writer){
	int i, j, k;
	int terminate = FALSE;
	int *v_str_len = (int*)malloc(sizeof(int)*n_tree->num_v);
	for(i=0; i<n_tree->num_v; i++)
		if(n_tree->v_type[i] == V_TYPE_INIT)
			v_str_len[i] = 4;
		else
			v_str_len[i] = -1;
	while(terminate == FALSE){
		terminate = TRUE;
		for(i=0; i<n_tree->num_v; i++){
			if(n_tree->v_type[i] == V_TYPE_UNINIT){
				terminate = FALSE;
				n_tree->v_string[i] = (char*)malloc(sizeof(char)*MAX_NW_LEN);
				//two neighbors are initiated
				if((n_tree->e_type[i][0]+n_tree->e_type[i][1]+n_tree->e_type[i][2])==2){
					int to[2];
					for(k=0; k<2; k++){
						int c = k==0?0:1;
						for(j=0; j<3; j++){
							if(n_tree->e_type[i][j] == E_TYPE_UNINIT && 
									n_tree->v_type[n_tree->edges[i][j]] == V_TYPE_INIT){
								to[c] = n_tree->edges[i][j];
								n_tree->e_type[i][j] = E_TYPE_INIT;
							}
						}
					}
					sprintf(n_tree->v_string[i], "(%s,%s)", 
							n_tree->v_string[to[0]], 
							n_tree->v_string[to[1]]);
					v_str_len[i] = v_str_len[to[0]] + v_str_len[to[1]] + 3;
				}
				//three neighbors are initiated
				else if((n_tree->e_type[i][0]+n_tree->e_type[i][1]+n_tree->e_type[i][2])==3){
					sprintf(n_tree->v_string[i], "(%s,%s,%s)", 
							n_tree->v_string[n_tree->edges[i][0]], 
							n_tree->v_string[n_tree->edges[i][1]],
							n_tree->v_string[n_tree->edges[i][2]]);
					v_str_len[i] = v_str_len[n_tree->edges[i][0]] + 
						v_str_len[n_tree->edges[i][1]] + 
						v_str_len[n_tree->edges[i][2]] + 3;
				}
				n_tree->v_type[i] = V_TYPE_UNINIT;
			}
		}
	}
	//find the largest v_string and print it out
	int max_len = 0, max_idx = -1;
	for(i=0; i<n_tree->num_v; i++)
		if(v_str_len[i] > max_len){
			max_len = v_str_len[i];
			max_idx = i;
		}
	fprintf(writer, "%s;\n", n_tree->v_string[max_idx]);
	//free
	free(v_str_len);
}

/*
 * this is used to turn pure edge info into a 
 * tree data structure.
 **/
void
from_edges_to_n_tree(pnt n_tree, tte *edges, int *map,
		int num_input_genome, int num_edge){
	int i, j, k;
	n_tree->num_v = 2*num_input_genome -1;	
	n_tree->v_type = (int*)malloc(sizeof(int)*n_tree->num_v);
	n_tree->e_idx = (int*)malloc(sizeof(int)*n_tree->num_v);
	n_tree->edges = (int**)malloc(sizeof(int*)*n_tree->num_v);
	n_tree->e_type = (int**)malloc(sizeof(int*)*n_tree->num_v);
	n_tree->v_string = (char**)malloc(sizeof(char*)*n_tree->num_v);
	for(i=0; i<n_tree->num_v; i++){
		n_tree->e_idx[i] = 0;
		n_tree->edges[i] = (int*)malloc(sizeof(int)*3);
		n_tree->e_type[i] = (int*)malloc(sizeof(int)*3);
		for(j=0; j<3; j++)
			n_tree->e_type[i][j] = E_TYPE_UNINIT;
	}

	for(i=0; i<num_edge; i++){
		int from = edges[i].from;
		int to = edges[i].to;
		//vertex
		if(from < num_input_genome && 
				n_tree->v_type[from] != V_TYPE_INIT){
			n_tree->v_type[from] = V_TYPE_INIT;
			n_tree->v_string[from] = (char*)malloc(sizeof(char)*MAX_NW_LEN);
			sprintf(n_tree->v_string[from], "%d", map[from]);
		}
		else if(from >= num_input_genome && 
			n_tree->v_type[from] != V_TYPE_UNINIT)
			n_tree->v_type[from] = V_TYPE_UNINIT;
		if(to < num_input_genome && 
			n_tree->v_type[to] != V_TYPE_INIT){
			n_tree->v_type[to] = V_TYPE_INIT;
			n_tree->v_string[to] = (char*)malloc(sizeof(char)*MAX_NW_LEN);
			sprintf(n_tree->v_string[to], "%d", map[to]);
		}
		else if(to >= num_input_genome && 
			n_tree->v_type[to] != V_TYPE_UNINIT)
			n_tree->v_type[to] = V_TYPE_UNINIT;
		//edge
		int e_idx_from = n_tree->e_idx[from]++;
		int e_idx_to = n_tree->e_idx[to]++;
		n_tree->edges[from][e_idx_from] = to;	
		n_tree->edges[to][e_idx_to] = from;	
	}
}

/*
 *this is to free the n_tree data structure
 **/
void
free_n_tree(pnt n_tree){
	int i, j, k;
	for(i=0; i<n_tree->num_v; i++){
		free(n_tree->edges[i]);
		free(n_tree->e_type[i]);
	}
	free(n_tree->v_type);
	free(n_tree->e_idx);
	free(n_tree->edges);
	free(n_tree->e_type);
	free(n_tree->v_string);
	free(n_tree);
}

void
from_newick_to_edge(char *file, pt tree){
	int has_one_edge = TRUE;
	char *nw_string = (char*)malloc(sizeof(char)*MAX_NW_LEN);
	FILE *reader = fopen(file, "r");
	int nw_len = readline(reader, nw_string, MAX_NW_LEN);
	nw_string[nw_len] = '\0';
	fclose(reader);
	while(has_one_edge == TRUE){
		int start = -1;
		int end = -1;
		has_one_edge = find_pattern(nw_string, "([0-9]*,[0-9]*)", &start, &end);
		if(has_one_edge == TRUE)
			from_nwstring_to_two_edge(tree, nw_string, start, end, nw_len);
	}
	int has_three_edges = TRUE;
	while(has_three_edges == TRUE){
		int start = -1;
		int end = -1;
		has_three_edges = find_pattern(nw_string, "([0-9]*,[0-9]*,[0-9]*)", &start, &end);
		if(has_three_edges == TRUE)
			from_nwstring_to_three_edges(tree, nw_string, start, end, nw_len);
	}
}

int
find_pattern(char *nw_string, char *pattern,
		int *start, int *end){
	int i, j, k;
	int res;
	int len;
	char err_buf[MAX_NW_LEN];
	int num_int;
	regex_t preg;
	regmatch_t pmatch[2];
	if( (res = regcomp(&preg, pattern, REG_EXTENDED)) != 0){
		regerror(res, &preg, err_buf, BUFSIZ);
		printf("regcomp: %s\n", err_buf);
		exit(res);
	}
	res = regexec(&preg, nw_string, 1, pmatch, REG_NOTBOL);
	if(res == REG_NOMATCH)
		return FALSE;
	*start = pmatch[0].rm_so;
	*end = pmatch[0].rm_eo;
	regfree(&preg);
	return TRUE;
} 

void
from_nwstring_to_two_edge(pt tree, char *nw_string,
		int start, int end, int len){
	char substr[MAX_NW_LEN];
	memcpy(substr, nw_string+start, (end-start));
	int res;
        char err_buf[MAX_NW_LEN];
        int num_int;
        regex_t preg;
        regmatch_t pmatch[2];
	char *pattern = "[0-9]*";
        if( (res = regcomp(&preg, pattern, REG_EXTENDED)) != 0){
                regerror(res, &preg, err_buf, BUFSIZ);
                printf("regcomp: %s\n", err_buf);
                exit(res);
        }
        res = regexec(&preg, nw_string, 2, pmatch, REG_NOTBOL);
	int from = nw_string_to_int(substr, pmatch[0].rm_so,
                pmatch[0].rm_eo);
	int to = nw_string_to_int(substr, pmatch[1].rm_so,
                pmatch[1].rm_eo);
	int med_genome = tree->orders->med_genome_idx++;
	tree->edges[tree->edge_idx].from = from;
	tree->edges[tree->edge_idx].to = med_genome;
	tree->edge_idx++;
	tree->edges[tree->edge_idx].from = to;
	tree->edges[tree->edge_idx].to = med_genome;
	tree->edge_idx++;
	replace_nwstring_by_int(nw_string, start, end, len, med_genome);
        regfree(&preg);
}

void
from_nwstring_to_three_edges(pt tree, char *nw_string,
		int start, int end, int len){
	char substr[MAX_NW_LEN];
	memcpy(substr, nw_string+start, (end-start));
	int res;
        char err_buf[MAX_NW_LEN];
        int num_int;
        regex_t preg;
        regmatch_t pmatch[3];
	char *pattern = "[0-9]*";
        if( (res = regcomp(&preg, pattern, REG_EXTENDED)) != 0){
                regerror(res, &preg, err_buf, BUFSIZ);
                printf("regcomp: %s\n", err_buf);
                exit(res);
        }
        res = regexec(&preg, nw_string, 3, pmatch, REG_NOTBOL);
	int one = nw_string_to_int(substr, pmatch[0].rm_so,
                pmatch[0].rm_eo);
	int two = nw_string_to_int(substr, pmatch[1].rm_so,
                pmatch[1].rm_eo);
	int three = nw_string_to_int(substr, pmatch[2].rm_so,
                pmatch[2].rm_eo);
	int med_genome = tree->orders->med_genome_idx++;
	tree->edges[tree->edge_idx].from = one;
	tree->edges[tree->edge_idx].to = med_genome;
	tree->edge_idx++;
	tree->edges[tree->edge_idx].from = two;
	tree->edges[tree->edge_idx].to = med_genome;
	tree->edge_idx++;
	tree->edges[tree->edge_idx].from = three;
	tree->edges[tree->edge_idx].to = med_genome;
	tree->edge_idx++;
        regfree(&preg);
}

int
nw_string_to_int(char *nw_string, int start, 
		int end){
	int i, j, k;
	char buf[100];
	for(i=start, j=0; i<end; i++, j++)
		buf[j] = nw_string[i]; 
	buf[j]='\0';
	return atoi(buf);
}

void
replace_nwstring_by_int(char *nw_string, int start,
		int end, int len, 
		int val){
	int i, j, k;
	char buf_head[MAX_NW_LEN];
	char buf_tail[MAX_NW_LEN];
	for(i=0; i<start; i++)
		buf_head[i] = nw_string[i];
	buf_head[i] = '\0';
	for(i=end,j=0; i<len; i++,j++)
		buf_tail[j] = nw_string[i];
	buf_head[j] = '\0';
	sprintf(nw_string, "%s%d%s",buf_head, val, buf_tail);
}

int 
readline(FILE *f, char *buffer, 
		size_t len){
	char c; 
	int i;

	memset(buffer, 0, len);
	for (i = 0; i < len; i++){   
		int c = fgetc(f); 
		if (!feof(f)){   
			if (c == '\r')
				buffer[i] = 0;
			else if (c == '\n'){   
				buffer[i] = 0;
				return i+1;
			}   
			else
				buffer[i] = c; 
		}   
		else   
			return -1; 
	}   
	return -1; 
}
