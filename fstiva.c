/* Radu Cosmin - 313 CB */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "structures.h"

//Initializare stiva cu elemente ce au dimensiunea d
void* inits(size_t d)
{
	ASt s;
	//Alocare + testare alocare
	s = (ASt)calloc(1, sizeof(TStiva));
	if(!s)
	{
		return NULL;
	}
	s->dime = d;
	return (void*)s;
}

//Introducere in stiva
int push(void* s, void* ae)
{
	ACel aux = (ACel)calloc(1, sizeof(TCel));
	if (!aux)
	{
		return 0;
	}
	//Setare pointer catre un element deja alocat
	aux->info = ae;
	//Restabilire legaturi in lista generica
	aux->urm = VF(s);
	VF(s) = aux;
	return 1;
}

//Functie pentru extragerea elementului de la varful stivei
int pop(void* s, void** ae)
{
	if (VF(s) == NULL)
	{
		return 0;
	}
	ACel aux = VF(s);
	//Stabilire unui pointer catre elemntul extras
	*ae = aux->info;
	VF(s) = aux->urm;
	free(aux);
	return 1;
}

//Resetare stiva ce contine pagini web
void reset_sw(void* s)
{
	ACel l;
	l = (ACel)VF(s);
	distr(&l, distr_web_page);
	VF(s) = NULL;
}

//Distrugere stiva de pagini web
void distr_sw(void* s)
{
	reset_sw(s);
	free(s);
}