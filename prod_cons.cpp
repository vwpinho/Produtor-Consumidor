#include <pthread.h>
#include <list>
#include <iostream>

using namespace std;

// Estrutura do argumento a ser passado pelos threads 
typedef struct{
    list<int> *l;
    int v,t;
}thread_arg, *ptr_thread_arg;

// Definicoes iniciais importantes
pthread_t *threads_p;
pthread_t *threads_c;
thread_arg* arguments;
pthread_mutex_t mutex_buffer;
pthread_cond_t not_empty_buffer;
pthread_cond_t not_full_buffer;

// Assinaturas das funcoes do produtor e do consumidor
void *prod(void * args);
void *con(void * args);

// Funcao principal
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

    // Aloca os threads que vao ser usados
    threads_p = (pthread_t*)malloc(p*sizeof(pthread_t));
    threads_c = (pthread_t*)malloc(c*sizeof(pthread_t));
    // Aloca o argumento a ser passado para os threads
    arguments = (ptr_thread_arg)malloc(sizeof(thread_arg));
    arguments[0].l = &lista;
    arguments[0].v = v;
    arguments[0].t = t;

    // Inicializa o mutex e as variaveis de condicao
    pthread_cond_init(&not_empty_buffer,NULL);
    pthread_cond_init(&not_full_buffer,NULL);
    pthread_mutex_init(&mutex_buffer,NULL);

    cout << "Processando..." << endl;
    
    // Cria os produtores
    for(int i=0;i<p;i++){
        pthread_create(&(threads_p[i]), NULL, prod, &(arguments[0]));
    }
    // Cria os consumidores
    for(int i=0;i<c; i++){
        pthread_create(&(threads_c[i]),NULL,con,&(arguments[0]));
    }
    // Aguarda o termino de todos os produtores
    for(int i=0;i<p;i++){
        pthread_join(threads_p[i],NULL);
    }
    // Insere tantos -1 quanto o numero de consumidores
    // E envia um sinal que a lista nao esta vazia
    for(int i=0;i<c;i++){
        arguments[0].l->push_back(-1);
        pthread_cond_signal(&not_empty_buffer);
    }
    // Aguarda o termino dos consumidores
    for(int i=0;i<c;i++){
        pthread_join(threads_c[i],NULL);
    }
}

// Funcao produtora
void *prod(void * args){
    
    ptr_thread_arg a = (ptr_thread_arg)args;
  
    // Insere tantos valores quanto os passados por arg
    for(int i=0;i<a->v;i++){
        // Da lock no mutex pois esta entrando na secao critica
        pthread_mutex_lock(&mutex_buffer);
        // Testa se o buffer esta cheio
        while(a->l->size() == a->t){
                // Se estiver cheio aguarda o sinal de que nao esta mais cheio
                pthread_cond_wait(&not_full_buffer,&mutex_buffer);
        }
        // Se nao estiver, insere um valor randomico
        a->l->push_back(rand());
        // Envia um sinal que o buffer nao esta vazio
        pthread_cond_signal(&not_empty_buffer);
        // Remove o lock do mutex para que outras threads possa usa-lo
        pthread_mutex_unlock(&mutex_buffer);
    } 
    return NULL;
}

// Funcao consumidora
void *con(void * args){
    ptr_thread_arg a = (ptr_thread_arg)args;
    // Laco infinito
    while(1){
        int aux,flag;
        // Da lock no mutex pois esta entrando na secao critica
        pthread_mutex_lock(&mutex_buffer);
        // Testa se o buffer nao esta vazio
        while(a->l->size() == 0){
            // Se estiver vazio aguarda o sinal de que nao esta mais vazio
            pthread_cond_wait(&not_empty_buffer,&mutex_buffer);
        }
        // Remove o primeiro elemento a ser inserido no buffer
        aux = a->l->front();
        a->l->pop_front();
        // Se o elemento for -1, libera o mutex e termina a thread
        if(aux==-1){
            pthread_mutex_unlock(&mutex_buffer);
            break;
        }
        // Verifica se o elemento e primo
        flag = 1;
        for(int i = aux-1;i>1;i--){
            if(aux%i == 0){
                flag=0;
                break;
            }
        }
        // Se for printa o elemento primo
        if(flag){
            cout << "[" << pthread_self() << "," << aux << "]" << endl;
        }
        // Envia um sinal de que o buffer nao esta mais cheio
        pthread_cond_signal(&not_full_buffer);
        // Libera o mutex para as outra threads
        pthread_mutex_unlock(&mutex_buffer);
    }
    return NULL;
}