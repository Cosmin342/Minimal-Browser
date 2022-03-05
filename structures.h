/* Radu Cosmin - 313 CB */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "utils.h"

#ifndef __STRUCTURES__
#define __STRUCTURES__

#define VF(a) (((ASt)(a))->vf)
#define DIME(a) (((ASt)(a))->dime)
#define DIMEQ(a) (((AQ)(a))->dime)
#define IC(a) (((AQ)(a))->ic)
#define SC(a) (((AQ)(a))->sc)

#define NEW_COM			2
#define DEL_COM			3
#define CHANGE_COM		4
#define PRINT_COM		5
#define GOTO_COM		6
#define BACK_COM		7
#define FORWARD_COM		8
#define DEL_HIST_COM	9
#define HISTC_COM		10
#define LIST_COM		11
#define LDOWN_COM		12
#define DOWN_COM		13
#define WAIT_COM		14
#define URL_MAX			21
#define MAX_LINE		256
#define DEF_BAND		1024

//Definire pointer la o functie de tip void ce primeste un void*
typedef void (*TFD)(void*);

//Structura unei celule a listei generice pt stiva sau coada
typedef struct cel{
	struct cel *urm;
	void* info;
} TCel, *ACel;

//Structura coada
typedef struct coada{
	size_t dime;
	ACel ic, sc;
} TCoada, *AQ;

//Structura stiva
typedef struct stiva{
	size_t dime;
	ACel vf;
} TStiva, *ASt;

//Structura unei pagini web
typedef struct{
	char url[21];
	int num_res;
	TR resources;
} t_web_page, *TWP;

//Structura listei circulare de taburi
typedef struct tab{
	struct tab *pre, *urm;
	TWP current_page;
	ASt back_stack;
	ASt forward_stack;
} t_tabs, *TAB;

//Structura browser-ului
typedef struct{
	TAB tabs;
	AQ history;
	AQ downloads;
} t_browser, *TB;

//Functie pentru initializarea stivei
void* inits(size_t d);

//Introducere element in stiva
int push(void* s, void* ae);

//Extragere element din stiva
int pop(void* s, void** ae);

//Functie pentru stergerea unei pagini web
void distr_web_page(void* info);

//Functie pentru stergerea unei resurse
void distr_resource(void* info);

//Distrugerea listei unei stive/cozi folosind functia dist
void distr(ACel* list, TFD dist);

//Resetare stiva de pagini web
void reset_sw(void* s);

//Stergere stiva de pagini web
void distr_sw(void* s);

//Functie pentru initializarea cozii
void* initq(size_t d);

//Introducere element in coada
int intrq(void* q, void* ae);

//Extragere element in coada
int extrq(void* q, void** ae);

//Resetare coada de pagini web
void reset_qw(void* q);

//Distrugere coada de pagini web
void distr_qw(void* q);

//Distrugere coada de resurse
void distr_qr(void* q);

//Functie pentru deschiderea browser-ului
TB deschide();

//Setare bandwidth
unsigned long set_band(char* comanda);

//Inserare nou tab
int ins_tab(TAB tabs);

//Stergerea ultimului tab deschis
void del_tab(TAB *tabs);

//Schimbarea tab-ului curent
TAB change_tab(char* comanda, TB browser);

//Deschiderea unei pagini web
void open_page(char* comanda, TAB current_tab, TB browser);

//Afisarea tab-urilor deschise
void print_open_tabs(TB browser, FILE* file);

//Stergerea memoriei ocupate de browser
void sterge(TB *browser);

//Intoarcere la o pagina anterioara
void back(TAB current_tab);

//Intoarcere la o pagina anterioara operatiei de back
void forward(TAB current_tab);

//Afisarea istoricului
void history(TB browser, FILE *file);

//Stergerea de intrari din istoric
void delete_history(TB browser, int intr);

//Afisarea resurselor descarcabile dintr-o pagina web
void show_resources(TWP web_page, FILE* file);

//Adaugarea unei resurse in coada de prioritati
void download(TR resources, int nr, TB browser);

//Afisarea resurselor descarcate si a celor in curs de descarcare
void downloads(AQ resources, AQ completed, FILE* file);

//Simulare descarcare
void desc_partial(unsigned long bandwidth, AQ downloads, AQ completed);

#endif