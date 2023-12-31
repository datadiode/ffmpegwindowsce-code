/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"
#include "log.h"
#include "tree.h"

typedef struct AVTreeNode{
    struct AVTreeNode *child[2];
    void *elem;
    int state;
}AVTreeNode;

void *av_tree_find(const AVTreeNode *t, void *key, int (*cmp)(void *key, const void *b), void *next[2]){
    if(t){
        unsigned int v= cmp(t->elem, key);
        if(v){
            if(next) next[(v>>31)^1]= t->elem;
            return av_tree_find(t->child[v>>31], key, cmp, next);
        }else{
            return t->elem;
        }
    }
    return 0;
}

void *av_tree_insert(AVTreeNode **tp, void *key, int (*cmp)(void *key, const void *b)){
    AVTreeNode *t= *tp;
    if(t){
        unsigned int v= cmp(t->elem, key);
        if(v){
            int i= v>>31;
            AVTreeNode **child= &t->child[i];
            void *ret= av_tree_insert(child, key, cmp);
            if(!ret){
                t->state -= ((int)v>>31)|1;
                if(!(t->state&1)){
                    if(t->state){
                        if((*child)->state*2 == t->state){
                            *tp= *child;
                            *child= (*child)->child[i^1];
                            (*tp)->child[i^1]= t;
                            t->state= 0;
                        }else{
                            *tp= (*child)->child[i^1];
                            (*child)->child[i^1]= (*tp)->child[i];
                            (*tp)->child[i]= *child;
                            *child= (*tp)->child[i^1];
                            (*tp)->child[i^1]= t;

                            i= (*tp)->state > 0;
                            (*tp)->child[i  ]->state= 0;
                            (*tp)->child[i^1]->state= -(*tp)->state;
                        }
                        (*tp)->state=0;
                    }
                    return key;
                }
            }
            return ret;
        }else{
            return t->elem;
        }
    }else{
        *tp= av_mallocz(sizeof(AVTreeNode));
        (*tp)->elem= key;
        return 0;
    }
}

void av_tree_destroy(AVTreeNode *t){
    av_tree_destroy(t->child[0]);
    av_tree_destroy(t->child[1]);
    av_free(t);
}

#if 0
void av_tree_enumerate(AVTreeNode *t, void *opaque, int (*f)(void *opaque, void *elem)){
    int v= f(opaque, t->elem);
    if(v>=0) av_tree_enumerate(t->child[0], opaque, f);
    if(v<=0) av_tree_enumerate(t->child[1], opaque, f);
}
#endif

#ifdef TEST

static int check(AVTreeNode *t){
    if(t){
        int left= check(t->child[0]);
        int right= check(t->child[1]);

        if(left>999 || right>999)
            return 1000;
        if(right - left != t->state)
            return 1000;
        if(t->state>1 || t->state<-1)
            return 1000;
        return FFMAX(left, right)+1;
    }
    return 0;
}

static void print(AVTreeNode *t, int depth){
    int i;
    for(i=0; i<depth*4; i++) av_log(NULL, AV_LOG_ERROR, " ");
    if(t){
        av_log(NULL, AV_LOG_ERROR, "Node %p %2d %4d\n", t, t->state, t->elem);
        print(t->child[0], depth+1);
        print(t->child[1], depth+1);
    }else
        av_log(NULL, AV_LOG_ERROR, "NULL\n");
}

int cmp(const void *a, const void *b){
    return a-b;
}

int main(){
    int i,j,k;
    AVTreeNode *root= NULL;

    for(i=0; i<10000; i++){
        int j= (random()%863294);
        if(check(root) > 999){
            av_log(NULL, AV_LOG_ERROR, "FATAL error %d\n", i);
        print(root, 0);
            return -1;
        }
        av_log(NULL, AV_LOG_ERROR, "inserting %4d\n", j);
        av_tree_insert(&root, (void*)(j+1), cmp);
    }
    return 0;
}
#endif
