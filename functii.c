/* Radu Cosmin - 313 CB */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "structures.h"

//Functie pentru distrugerea unei pagini web
void distr_web_page(void* info)
{
	TWP page = (TWP)info;
	//Eliberare vector de resurse si a structurii pentru pagina
	free(page->resources);
	free(page);
	page = NULL;
}

//Distrugerea unei resurse
void distr_resource(void* info)
{
	TR resource = (TR)info;
	free(resource);
	resource = NULL;
}

/*
Distrugerea listei generice a unei stive/cozi utilizand poinetrul la functie de tip TFD
*/
void distr(ACel* list, TFD dist)
{
	ACel aux;
	//Parcurgerea listei si eliberarea memoriei ocupate de fiecare element
	while(*list)
	{
		aux = *list;
		*list = (*list)->urm;
		if (aux->info != NULL)
		{
			dist(aux->info);
		}
		free(aux);
	}
}

//Initializare lista de taburi
TAB init_tabs()
{
	//Alocare santinela
	TAB tabs = (TAB)calloc(1, sizeof(t_tabs));
	if (!tabs)
	{
		return NULL;
	}
	//Setare campuri pre si urm la santinela
	tabs->pre = tabs;
	tabs->urm = tabs;
	return tabs;
}

//Inserare nou tab in lista de taburi
int ins_tab(TAB tabs)
{
	//Alocare nou tab, back stack si forward stack + testari
	TAB aux = (TAB)calloc(1, sizeof(t_tabs));
	if (!aux)
	{
		return 0;
	}
	ASt back_stack = (ASt)inits(sizeof(t_web_page));
	if (!back_stack)
	{
		free(aux);
		return 0;
	}
	ASt forward_stack = (ASt)inits(sizeof(t_web_page));
	if (!forward_stack)
	{
		distr_sw(back_stack);
		free(aux);
		return 0;	
	}
	//Un nou tab nu va avea vreo pagina deschisa
	aux->current_page = NULL;
	aux->back_stack = back_stack;
	aux->forward_stack = forward_stack;
	//Legare propriu-zisa in lista de tab-uri.
	aux->urm = tabs;
	aux->pre = tabs->pre;
	tabs->pre->urm = aux;
	tabs->pre = aux;
	return 1;
}

//Stergerea ultimului tab
void del_tab(TAB *tabs)
{
	//Extragerea ultimului tab + refacerea legaturilor
	TAB aux = (*tabs)->pre;
	aux->pre->urm = *tabs;
	(*tabs)->pre = aux->pre;
	//Distrugerea stivelor asociate tab-ului
	distr_sw(aux->back_stack);
	distr_sw(aux->forward_stack);
	//Daca tab-ul are o pagina deschisa, atunci si aceasta va fi stearsa
	if (aux->current_page != NULL)
	{
		free(aux->current_page->resources);
		free(aux->current_page);
	}
	free(aux);
}

TB deschide()
{
	//Alocare structura de browser + testare
	TB browser = (TB)calloc(1, sizeof(t_browser));
	if (!browser)
	{
		return NULL;
	}
	//Alocarea cozilor browser-ului
	browser->history = (AQ)initq(sizeof(t_web_page));
	if (!browser->history)
	{
		free(browser);
		return NULL;
	}
	browser->downloads = (AQ)initq(sizeof(Resource));
	if (!browser->downloads)
	{
		distr_qw(browser->history);
		free(browser);
		return NULL;
	}
	//Initializarea listei de tab-uri + inserarea primului tab
	browser->tabs = init_tabs();
	ins_tab(browser->tabs);
	return browser;
}

//Extragerea bandwidth-ului dintr-un sir
char* banda(char* comanda)
{
	char* token;
	token = strtok(comanda, " ");
	token = strtok(NULL, " ");
	return token;
}

//Setare bandwidth
unsigned long set_band(char* comanda)
{
	unsigned long band;
	//Convertire char* la unsigned long
	band = atol(banda(comanda));
	return band;
}

//Schimbare tab
TAB change_tab(char* comanda, TB browser)
{
	int tab, i;
	//Extragere numar tab
	char* token;
	token = strtok(comanda, " ");
	token = strtok(NULL, " ");
	tab = atoi(token);
	//Parcurgere lista de tab-uri pana se ajunge la tab-ul corespunzator
	TAB current_tab = browser->tabs->urm;
	for (i = 0; i < tab; i++)
	{
		current_tab = current_tab->urm;
	}
	return current_tab;
}

//Deschiderea unei noi pagini web
void open_page(char* comanda, TAB current_tab, TB browser)
{
	char *token, url[URL_MAX];
	//Extragerea url-ului paginii
	token = strtok(comanda, " ");
	token = strtok(NULL, " ");
	strcpy(url, token);
	//Alocare pagina + testare alocare
	TWP page = (TWP)calloc(1, sizeof(t_web_page));
	if (!page)
	{
		return;
	}
	strcpy(page->url, url);
	//Obtinerea vectorului de resurse si a numarului de resurse
	page->resources = get_page_resources(page->url, &(page->num_res));
	/*
	Daca tab-ul curent nu are pagini, campul current_page va pointa la
	noua pagina
	*/
	if (current_tab->current_page == NULL)
	{
		current_tab->current_page = page;
	}
	//Altfel, intai se va pune pagina curenta in stiva de back
	else
	{
		push((void*)current_tab->back_stack, (void*)current_tab->current_page);
		current_tab->current_page = page;
	}
	/*
	Se realizeaza o copie a paginii nou deschise ce urmeaza a fi pusa in
	coada de history a browser-ului
	*/
	TWP page_hist = (TWP)calloc(1, sizeof(t_web_page));
	if (!page_hist)
	{
		free(page->resources);
		free(page);
	}
	strcpy(page_hist->url, current_tab->current_page->url);
	page_hist->resources = get_page_resources(page_hist->url,
		&(page_hist->num_res));
	intrq((void*)browser->history, (void*)page_hist);
}

//Afisarea tab-urilor deschise dintr-un browser
void print_open_tabs(TB browser, FILE* file)
{
	TAB tabs;
	int i = 0;
	//Parcurgere lista de tab-uri
	for (tabs = browser->tabs->urm; tabs != browser->tabs; tabs = tabs->urm)
	{
		fprintf(file, "(%d: ", i);
		//Daca tab-ul nu pointeaza la nicio pagina, se va afisa empty
		if (tabs->current_page == NULL)
		{
			fprintf(file, "empty)\n");
		}
		else
		{
			fprintf(file, "%s)\n", tabs->current_page->url);
		}
		i++;
	}
}

//Intoarcere la o pagina anterioara
void back(TAB current_tab)
{
	//Daca nu exista pagini anterioare, se afiseaza un mesaj de eroare
	if (VF(current_tab->back_stack) == NULL)
	{
		printf("%s\n", "can’t go back, no pages in stack");
	}
	/*
	Altfel se extrage pagina anterioara, se pune in back_stack actuala pagina
	si se atribuie pointerului din tab vechea pagina
	*/
	else
	{
		TWP page = NULL;
		pop((void*)current_tab->back_stack, (void*)&page);
		push((void*)current_tab->forward_stack,
			(void*)(current_tab->current_page));
		current_tab->current_page = page;
	}
}

//Intoarcere la o pagina anterioara operatiei de back
void forward(TAB current_tab)
{
	//Daca nu exista pagini in forward, se afiseaza un mesaj de eroare
	if (VF(current_tab->forward_stack) == NULL)
	{
		printf("%s\n", "can’t go forward, no pages in stack");
	}
	/*
	Altfel se extrage pagina anterioara din forward_stack, se pune in
	back_stack actuala pagina si se atribuie pointerului din tab vechea pagina
	*/
	else
	{
		TWP page = NULL;
		pop((void*)current_tab->forward_stack, (void*)&page);
		push((void*)current_tab->back_stack,
			(void*)(current_tab->current_page));
		current_tab->current_page = page;
	}
}

//Afisare istoric
void history(TB browser,FILE *file)
{
	//Coada unde se pun paginile afisate
	AQ new_history = initq(sizeof(t_web_page));
	TWP page = NULL;
	//Se extrage fiecare pagina si se afiseaza url-ul ei
	while (extrq((void*)browser->history, (void*)&page) == 1)
	{
		fprintf(file, "%s\n", page->url);
		intrq((void*)new_history, (void*)page);
	}
	distr_qw(browser->history);
	browser->history = new_history;
}

//Stergerea a intr intrari din istoric sau a tuturor
void delete_history(TB browser, int intr)
{
	int i;
	//Daca intr e 0, coada de history va fi golita
	if (intr == 0)
	{
		reset_qw(browser->history);
		return;
	}
	//Altfel, se extrag pe rand intr pagini si se elibereaza memoria lor
	for (i = 0; i < intr; i++)
	{
		TWP page = NULL;
		extrq((void*)browser->history, (void*)&page);
		if (page != NULL)
		{
			if (page->resources != NULL)
			{
				free(page->resources);
			}
			free(page);
		}
	}
}

//Afisarea resurselor descarcabile dintr-o pagina web
void show_resources(TWP web_page, FILE* file)
{
	int i;
	//Daca nu exista resurse, se iese din functie
	if (web_page == NULL)
	{
		return;
	}
	//Altfel se parcurge vectorul de resurse si se printeaza conform formatului
	for (i = 0; i < web_page->num_res; i++)
	{
		fprintf(file, "[%d - \"%s\" : %lu]\n", i, web_page->resources[i].name,
			web_page->resources[i].dimension);
	}
}

//Adaugarea unei resurse in coada de prioritati
void download(TR resources, int nr, TB browser)
{
	//Copie a resursei de introdus
	TR resource = (TR)calloc(1, sizeof(Resource));
	if (!resource)
	{
		return;
	}
	//Element folosit pt extragerea din coada
	TR elem = NULL;
	unsigned long ramas1, ramas2;
	//Coada auxiliara pt ordonare
	AQ aux = initq(sizeof(Resource));
	if (!aux)
	{
		free(resource);
		return;
	}
	//Copierea propriu zisa a elementelor structurii
	strcpy(resource->name, resources[nr].name);
	resource->dimension = resources[nr].dimension;
	//Daca nu exista vreun element in coada, se adauga resursa
	if (IC(browser->downloads) == NULL)
	{
		intrq((void*)browser->downloads, (void*)resource);
	}
	else
	{
		//Extragere pana la gasirea locului unde trebuie pusa resursa
		while (extrq((void*)browser->downloads, (void*)&elem) == 1)
		{
			//Cat a ramas de descarcat din elementul extras
			ramas1 = elem->dimension - elem->currently_downloaded;
			/*
			Daca este mai mica decat dimensiunea resursei, se verifica
			inceputul cozii
			*/
			if (ramas1 < resource->dimension)
			{
				if (IC(browser->downloads) != NULL)
				{
					//Cat a ramas de descarcat din elementul de la inceput
					ramas2 = ((TR)(IC(browser->downloads)->info))->dimension -
					((TR)(IC(browser->downloads)->info))->currently_downloaded;
					/*
					Daca este mai mare decat dimensiunea resursei, elem si
					resursa se introduc in coada auxiliara
					*/
					if (ramas2 > resource->dimension)
					{
						intrq((void*)aux, (void*)elem);
						intrq((void*)aux, (void*)resource);
						break;
					}
					//Altfel, este introdus doar elem
					else
					{
						intrq((void*)aux, (void*)elem);
					}
				}
				/*
				Daca nu mai exista elemente in coada, elem si resursa sunt
				introduse in coada auxiliara
				*/
				else
				{
					intrq((void*)aux, (void*)elem);
					intrq((void*)aux, (void*)resource);
					break;
				}
			}
			/*
			Daca nu este indeplinita conditia dintre ramas1 si dimensiunea
			resursei, se introduce resursa in coada auxiliara si apoi elem
			*/
			else
			{
				intrq((void*)aux, (void*)resource);
				intrq((void*)aux, (void*)elem);
				break;
			}
		}
		//Este golita coada initiala de descarcari
		while (extrq((void*)browser->downloads, (void*)&elem) == 1)
		{
			intrq((void*)aux, (void*)elem);
		}
		//La final continutul lui aux este mutat in coada initiala
		while (extrq((void*)aux, (void*)&elem) == 1)
		{
			intrq((void*)browser->downloads, (void*)elem);
		}
	}
	distr_qr(aux);
}

//Simuleaza descarcarea
void desc_partial(unsigned long bandwidth, AQ downloads, AQ completed)
{
	//Coada auxiliara
	AQ partial = initq(sizeof(Resource));
	if (!partial)
	{
		return;
	}
	TR elem = NULL;
	//Se extrag elementele din coada
	while (extrq((void*)downloads, (void*)&elem) == 1)
	{
		//Se calculeaza pentru fiecare element, cati B se descarca
		elem->currently_downloaded = elem->currently_downloaded + bandwidth;
		/*
		Daca s-au descarcat mai multi B decat dimensiunea resursei, ea va fi
		mutata in coada de descarcari finalizate
		*/
		if (elem->currently_downloaded >= elem->dimension)
		{
			//Bandwidth-ul ramas
			bandwidth = elem->currently_downloaded - elem->dimension;
			elem->currently_downloaded = elem->dimension;
			intrq((void*)completed, (void*)elem);
		}
		//Altfel, bandwidth-ul disponibil devine 0 pt restul resurselor
		else
		{
			bandwidth = 0;
			intrq((void*)partial, (void*)elem);
		}
	}
	//Elementele din coada auxiliara sunt mutate la loc in coada initiala
	while (extrq((void*)partial, (void*)&elem) == 1)
	{
		intrq((void*)downloads, (void*)elem);
	}
	distr_qr(partial);
}

//Afisarea resurselor descarcate si a celor in curs de descarcare
void downloads(AQ resources, AQ completed, FILE* file)
{
	TR elem = NULL;
	AQ res = initq(sizeof(Resource));
	while (extrq((void*)resources, (void*)&elem) == 1)
	{
		//Se afiseaza numele resursei si cat mai trebuie descarcat din aceasta
		fprintf(file, "[\"%s\" : %lu/%lu]\n", elem->name,
			elem->dimension - elem->currently_downloaded, elem->dimension);
		intrq((void*)res, (void*)elem);
	}
	while (extrq((void*)res, (void*)&elem) == 1)
	{
		intrq((void*)resources, (void*)elem);
	}
	while (extrq((void*)completed, (void*)&elem) == 1)
	{
		//Pentru cele terminate, se afiseaza completed
		fprintf(file, "[\"%s\" : completed]\n", elem->name);
		intrq((void*)res, (void*)elem);
	}
	while (extrq((void*)res, (void*)&elem) == 1)
	{
		intrq((void*)completed, (void*)elem);
	}
	distr_qr(res);
}

//Eliberarea memoriei ocupate de browser
void sterge(TB *browser)
{
	//Distrugerea cozilor
	distr_qw((*browser)->history);
	distr_qr((*browser)->downloads);
	TAB aux = (*browser)->tabs->urm;
	//Stergerea listei de tab-uri si a paginilor web asociate
	while (aux != (*browser)->tabs)
	{
		(*browser)->tabs->urm = (*browser)->tabs->urm->urm;
		distr_sw(aux->back_stack);
		distr_sw(aux->forward_stack);
		if (aux->current_page != NULL)
		{
			if (aux->current_page->resources != NULL)
			{
				free(aux->current_page->resources);
			}
			free(aux->current_page);
		}
		free(aux);
		aux = (*browser)->tabs->urm;
	}
	free(aux);
	//Eliberarea memoriei ocupate de structura de browser
	free(*browser);
}