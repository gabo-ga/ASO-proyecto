#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX 15

struct param {
	int num_estudiante;
	int num_grupo;
};

struct param buffer[MAX];

sem_t huecos,elementos,mute;

int pos_productor= 0;
int num_estudiantes, cantidad_grupos, plazas_por_grupo;
int *grupo_de_alumno;
int *plazas_libres;

void init_datos() {
	int i;
	grupo_de_alumno= (int*)malloc(sizeof(int)*num_estudiantes);
	for (i= 0; i< num_estudiantes; i++)
		grupo_de_alumno[i]= -1;
	plazas_libres= (int*)malloc(sizeof(int)*cantidad_grupos);
	for (i= 0; i< cantidad_grupos; i++)
		plazas_libres[i]= plazas_por_grupo;
}

int *decide_preferencias (void) {
	int *peticiones;
	int i,pos1,pos2,aux;
	peticiones= (int *) malloc(sizeof(int)*cantidad_grupos);
	for (i= 0; i< cantidad_grupos; i++)
		peticiones[i]= i;
	for (i= 0; i< cantidad_grupos; i++) {
		pos1= random() %cantidad_grupos;
		pos2= random() %cantidad_grupos;
		aux= peticiones[pos1];
		peticiones[pos1]= peticiones[pos2];
		peticiones[pos2]= aux;
	}
	return peticiones;
}

int grupo_asignado (int num_estudiante) {
	return (grupo_de_alumno[num_estudiante]);
}

int hay_plazas_libres (int num_grupo) {
	return (plazas_libres[num_grupo]>0);
}

void asignar_grupo (int estudiante, int grupo) {
	if (grupo_de_alumno[estudiante]==-1) {
		grupo_de_alumno[estudiante]= grupo;
		plazas_libres[grupo]--;
	}else {
		printf("Error inesperado\n");
		pthread_exit(NULL);
	}
}

void *productor(void *p) {
	int num_estudiante= *(int *)p;
	int i;
	int *peticiones;
	struct param dato;
	peticiones= decide_preferencias ();
	for (i= 0; i< cantidad_grupos; i++) {
		dato.num_estudiante= num_estudiante;
		dato.num_grupo= peticiones[i];
		printf ("El estudiante %d pide el grupo %d\n", num_estudiante, peticiones[i]);
		sem_wait(&huecos);
		sem_wait(&mute);
		buffer[pos_productor]= dato;
		pos_productor= (pos_productor+1)%MAX;
		sem_post(&mute);
		sem_post(&elementos);
		if (grupo_asignado(num_estudiante)!=-1)
			pthread_exit(NULL);
		}
	pthread_exit(NULL);
}

void *consumidor (void *p) {
	int pos_consumidor= 0;
	int asignados= 0;
	struct param dato;
	while (asignados< num_estudiantes) {
		sem_wait(&elementos);
		dato= buffer[pos_consumidor];
		pos_consumidor= (pos_consumidor+1)%MAX;
		sem_post(&huecos);
		if (hay_plazas_libres(dato.num_grupo)) {
			if (grupo_asignado(dato.num_estudiante)!=-1)
				printf("Peticion rechazada. Estudiante %d, grupo %d. El estudiante ya tiene grupo asignado\n",
				dato.num_estudiante,dato.num_grupo);
			else {
				asignar_grupo(dato.num_estudiante, dato.num_grupo);
				printf("Al estudiante %d se le asigna el grupo %d\n",dato.num_estudiante, dato.num_grupo);
				asignados++;
			}
		} else
			printf("Peticion rechazada. Estudiante %d, grupo %d. Grupo lleno\n", dato.num_estudiante,dato.num_grupo);
	}
	pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
	pthread_t *estudiantes, gestor;
	int i, *datos;
	if (argc!= 4) {
		printf("Error. Debe proporcionar tres parametros: Num. estudiantes, Num. de grupos y tamaño de grupo\n");
		exit(1);
	}
	num_estudiantes= atoi(argv[1]);
	cantidad_grupos= atoi(argv[2]);
	plazas_por_grupo= atoi(argv[3]);
	if (num_estudiantes> cantidad_grupos*plazas_por_grupo) {
		printf("Error. No hay plazas para todos los estudiantes\n");
		exit(1);
	}
	init_datos();
	datos= (int*) malloc(sizeof(int)*num_estudiantes);
	estudiantes= (pthread_t*) malloc(sizeof(pthread_t)*num_estudiantes);
	sem_init(&huecos, 0, MAX);
	sem_init(&elementos, 0, 0);
	sem_init(&mute, 0, 1);
	pthread_create (&gestor, NULL, consumidor, NULL);
	for (i= 0; i< num_estudiantes; i++) {
		datos[i]= i;
		pthread_create (&estudiantes[i], NULL, productor, &datos[i]);
	}
	pthread_join(gestor, NULL);
	return 0;
}
