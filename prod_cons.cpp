#include <pthread.h>
#include <list>
#include <iostream>

using namespace std;

typedef struct{
    list<int> *l;
    int v,t;
}thread_arg, *ptr_thread_arg;

pthread_t *threads_p;
pthread_t *threads_c;
thread_arg* arguments;
pthread_mutex_t mutex_buffer;
pthread_cond_t not_empty_buffer;
pthread_cond_t not_full_buffer;

void *prod(void * args);
void *con(void * args);
int main(){
    srand(time(NULL));
    int v,p,c,t;
    cout << "informe o numero de iteracoes, a quantidade de produtores, a quantidade de consumidores e o tamanho maximo do buffer:" << endl;
    cin >> v;
    cin >> p;
    cin >> c;
    cin >> t;
    list<int> lista;
    list<int>::iterator it;

    threads_p = (pthread_t*)malloc(p*sizeof(pthread_t));
    threads_c = (pthread_t*)malloc(c*sizeof(pthread_t));
    arguments = (ptr_thread_arg)malloc(sizeof(thread_arg));
    arguments[0].l = &lista;
    arguments[0].v = v;
    arguments[0].t = t;

    pthread_cond_init(&not_empty_buffer,NULL);
    pthread_cond_init(&not_full_buffer,NULL);
    pthread_mutex_init(&mutex_buffer,NULL);
    cout << "Processando..." << endl;
    for(int i=0;i<p;i++){
        pthread_create(&(threads_p[i]), NULL, prod, &(arguments[0]));
    }
    for(int i=0;i<c; i++){
        pthread_create(&(threads_c[i]),NULL,con,&(arguments[0]));
    }
    for(int i=0;i<p;i++){
        pthread_join(threads_p[i],NULL);
    }
 
    for(int i=0;i<c;i++){
        arguments[0].l->push_back(-1);
        pthread_cond_signal(&not_empty_buffer);
    }
    
    for(int i=0;i<c;i++){
        pthread_join(threads_c[i],NULL);
    }


}


void *prod(void * args){
    
    ptr_thread_arg a = (ptr_thread_arg)args;
  
    for(int i=0;i<a->v;i++){
        pthread_mutex_lock(&mutex_buffer);
        while(a->l->size() == a->t){
                pthread_cond_wait(&not_full_buffer,&mutex_buffer);
        }
        a->l->push_back(rand());
        pthread_cond_signal(&not_empty_buffer);
        pthread_mutex_unlock(&mutex_buffer);
    } 
    return NULL;
}

void *con(void * args){
    ptr_thread_arg a = (ptr_thread_arg)args;
    while(1){
        int aux,flag;
        pthread_mutex_lock(&mutex_buffer);
        while(a->l->size() == 0){
            pthread_cond_wait(&not_empty_buffer,&mutex_buffer);
        }
        aux = a->l->front();
        a->l->pop_front();
        if(aux==-1){
            pthread_mutex_unlock(&mutex_buffer);
            break;
        }
        flag = 1;
        for(int i = aux-1;i>1;i--){
            if(aux%i == 0){
                flag=0;
                break;
            }
        }
        if(flag){
            cout << "[" << pthread_self() << "," << aux << "]" << endl;
        }
        
        pthread_cond_signal(&not_full_buffer);
        pthread_mutex_unlock(&mutex_buffer);
    }
    return NULL;
}