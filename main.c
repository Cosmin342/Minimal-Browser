/* Radu Cosmin - 313 CB */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "structures.h"

/*
Functie ce intoarce un numar intre 1 si 14 pentru a sti ce comanda trebuie
executata, cautandu-se in sirul functia numele fiecarei comenzi
*/
int consola(char *functia)
{
	char command1[] = "set_band";
	char command2[] = "newtab";
	char command3[] = "deltab";
	char command4[] = "change_tab";
	char command5[] = "print_open_tabs";
	char command6[] = "goto";
	char command7[] = "back";
	char command8[] = "forward";
	char command9[] = "del_history";
	char command10[] = "history";
	char command11[] = "list_dl";
	char command12[] = "downloads";
	char command13[] = "download";
	char command14[] = "wait";
	if (strstr(functia, command1) != NULL) return 1;
	if (strstr(functia, command2) != NULL) return NEW_COM;
	if (strstr(functia, command3) != NULL) return DEL_COM;
	if (strstr(functia, command4) != NULL) return CHANGE_COM;
	if (strstr(functia, command5) != NULL) return PRINT_COM;
	if (strstr(functia, command6) != NULL) return GOTO_COM;
	if (strstr(functia, command7) != NULL) return BACK_COM;
	if (strstr(functia, command8) != NULL) return FORWARD_COM;
	if (strstr(functia, command9) != NULL) return DEL_HIST_COM;
	if (strstr(functia, command10) != NULL) return HISTC_COM;
	if (strstr(functia, command11) != NULL) return LIST_COM;
	if (strstr(functia, command12) != NULL) return LDOWN_COM;
	if (strstr(functia, command13) != NULL) return DOWN_COM;
	if (strstr(functia, command14) != NULL) return WAIT_COM;
	return 0;
}

/*
Functie ce determina al doilea parametru pentru comenzile cu un parametru numar
*/
int det_nr(char* comanda)
{
	char* token;
	token = strtok(comanda, " ");
	token = strtok(NULL, " ");
	int intr = atoi(token);
	return intr;
}

//Functie in care se fac prelucrarile asupra browser-ului
void executa(TB* browser, TAB* current_tab, unsigned long *band, char* comand,
	int com_nr, FILE* out, AQ *completed)
{
	int intr;
	switch(com_nr)
	{
		//In cazul unei comenzi gresite, se iese din functie
		case 0:
			return;
			break;
		case 1:
			//Se modifica bandwidth-ul
			*band = set_band(comand);
			break;
		case NEW_COM:
			//Se insereaza un nou tab si se seteaza tab-ul curent la acesta
			ins_tab((*browser)->tabs);
			*current_tab = (*browser)->tabs->pre;
			break;
		case DEL_COM:
			/*
			Daca tab-ul curent este setat la ultimul, atunci current_tab va
			fi setat la penultimul tab
			*/
			if (*current_tab == (*browser)->tabs->pre)
			{
				*current_tab = (*browser)->tabs->pre->pre;
			}
			//Se sterge ultimul tab din lista
			del_tab(&((*browser)->tabs));
			break;
		case CHANGE_COM:
			//Se modifica tab-ul curent
			*current_tab = change_tab(comand, *browser);
			break;
		case PRINT_COM:
			//Printare tab-uri deschise
			print_open_tabs(*browser, out);
			break;
		case GOTO_COM:
			/*
			Se modifica pagina tab-ului curent si se descarca band * 1 B
			*/
			open_page(comand, *current_tab, *browser);
			desc_partial(*band, (*browser)->downloads, *completed);
			break;
		case BACK_COM:
			//Intoarcere la pagina anterioara
			back(*current_tab);
			break;
		case FORWARD_COM:
			//Se trece la pagina anterioara operatiei de back, daca exista
			forward(*current_tab);
			break;
		case DEL_HIST_COM:
			//Se sterge tot istoricul (daca intr e 0) sau intr intrari
			intr = det_nr(comand);
			delete_history(*browser, intr);
			break;
		case HISTC_COM:
			//Printare istoric
			history(*browser, out);
			break;
		case LIST_COM:
			//Printare resurse descarcabile din pagina curenta
			show_resources((*current_tab)->current_page, out);
			break;
		case LDOWN_COM:
			//Printarea istoricului de descarcari
			downloads((*browser)->downloads, *completed, out);
			break;
		case DOWN_COM:
			//Descarcare resursa cu indicele intr in vectorul de resurse
			intr = det_nr(comand);
			if ((*current_tab)->current_page != NULL)
			{
				download((*current_tab)->current_page->resources, intr,
					*browser);
			}
			break;
		case WAIT_COM:
			/*
			Se descarca intr * band B care sunt distribuiti in functie
			de prioritate
			*/
			intr = det_nr(comand);
			desc_partial(intr * (*band), (*browser)->downloads, *completed);
			break;
	}
}

int main(int argc, char const *argv[])
{
	//Fisierele din care se citesc/in care se printeaza
	FILE* in = fopen(argv[argc - NEW_COM], "rt");
	FILE* out = fopen(argv[argc - 1], "wt");
	if (!in)
	{
		exit(EXIT_FAILURE);
	}
	if (!out)
	{
		exit(EXIT_FAILURE);
	}
	//Deschidere browser + testare
	TB browser = deschide();
	if (!browser)
	{
		fclose(in);
		fclose(out);
		exit(EXIT_FAILURE);
	}
	//Variabila pentru bandwidth
	unsigned long band = DEF_BAND;
	//Setare tab curent la singurul tab deschis
	TAB current_tab = browser->tabs->urm;
	AQ completed = initq(sizeof(Resource));
	if (!completed)
	{
		fclose(in);
		fclose(out);
		sterge(&browser);
	}
	/*
	Variabila ce retine pozitia curenta din fisierul de intrare folosita
	pentru a evita rularea de doua ori a ultimei comenzi
	*/
	long int poz = ftell(in);
	while(!feof(in))
	{
		//Variabila ce retine comanda de pe o linie
		char com[MAX_LINE], *token, *comand;
		fgets(com, MAX_LINE, in);
		/*
		Daca pozitia de acum este egala cu cea de dinainte, s-a ajuns la
		final. Prin urmare, while-ul este oprit
		*/
		if (poz == ftell(in))
		{
			break;
		}
		else
		{
			poz = ftell(in);
		}
		token = strtok(com, "\n");
		//Copie a comenzii
		comand = strdup(token);
		executa(&browser, &current_tab, &band, comand, consola(token), out,
			&completed);
		free(comand);
	}
	fclose(in);
	fclose(out);
	/*
	Eliberarea memoriei ocupate de browser si de coada de descarcari terminate
	*/
	distr_qr(completed);
	sterge(&browser);
	return 0;
}