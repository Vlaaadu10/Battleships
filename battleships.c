/* 314CA Stefan Alexandru Vladut */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* constante si definitii */
#define NUMAR_TIPURI_NAVA 5
#define DIM_ALFABET 37
#define EPS 0.000000000001L

/* structuri */
typedef struct {
	char tip_nava;
	int lungime;
	char orientare;
	int x_start;
	int y_start;
	int lovituri;
	int este_distrusa;
} nava_t;

typedef struct {
	int n_lin;
	int n_col;
	nava_t *nave;
	int nr_nave;
	int nave_ramase;
	int *harta_nave;
	unsigned char *zone_atacate;
} tabla_t;

typedef struct {
	char id;
	int lungime;
	const char *nume;
} info_nava_t;

typedef struct {
	long long lovituri_nimerite;
	long long lovituri_totale;
	int nave_per_meci;
} statistici_t;

/* informatii despre tipurile de nave */
static const info_nava_t info_nave[NUMAR_TIPURI_NAVA] = {
	{ 'S', 5, "Shinano"},
	{ 'Y', 4, "Yamato"},
	{ 'B', 3, "Belfast"},
	{ 'L', 2, "Laffey"},
	{ 'A', 1, "Albacore"}
};

/* determina indexul in vector pentru (x,y) */
static int indice_tabla(const tabla_t *tabla, int x, int y)
{
	return (x - 1) * tabla->n_col + (y - 1);
}

/* cauta informatii despre tipul de nava */
static const info_nava_t *info_tip(char tip_nava)
{
	int i;
	for (i = 0; i < NUMAR_TIPURI_NAVA; i++) {
		if (info_nave[i].id == tip_nava) {
			return &info_nave[i];
		}
	}
	return NULL;
}

/* cauta pozitia in vectorul de tipuri de nave in functie de litera */
static int pozitie_tip_nava(char tip_nava)
{
	int i;
	for (i = 0; i < NUMAR_TIPURI_NAVA; i++) {
		if (info_nave[i].id == tip_nava) {
			return i;
		}
	}
	return -1;
}

/* initializare tabla */
static int initializare_tabla(tabla_t *tabla, int n_lin, int n_col)
{
	int nr_poz = n_lin * n_col;
	int i;
	tabla->n_lin = n_lin;
	tabla->n_col = n_col;
	tabla->nr_nave = 0;
	tabla->nave_ramase = 0;

	tabla->harta_nave = (int *)malloc((size_t)nr_poz * sizeof(int));
	if (!(tabla->harta_nave)) {
		fprintf(stderr, "Eroare: malloc() a esuat.\n");
		return 0;
	}
	tabla->zone_atacate =
		(unsigned char *)calloc((size_t)nr_poz, sizeof(unsigned char));
	if (!(tabla->zone_atacate)) {
		fprintf(stderr, "Eroare: malloc() a esuat.\n");
		free(tabla->harta_nave);
		return 0;
	}
	tabla->nave = (nava_t *)malloc((size_t)nr_poz * sizeof(nava_t));
	if (!(tabla->nave)) {
		fprintf(stderr, "Eroare: malloc() a esuat.\n");
		free(tabla->harta_nave);
		free(tabla->zone_atacate);
		return 0;
	}
	for (i = 0; i < nr_poz; i++) {
		tabla->harta_nave[i] = -1;
	}

	return 1;
}

/* elibereaza memoria alocata unei table */
static void elibereaza_tabla(tabla_t *tabla)
{
	if (!tabla) {
		return;
	}
	free(tabla->harta_nave);
	free(tabla->zone_atacate);
	free(tabla->nave);

	tabla->harta_nave = NULL;
	tabla->zone_atacate = NULL;
	tabla->nave = NULL;
}

/* verificare daca o nava poate fi plasata la (x,y) cu orientarea data */
static int verificare_plasare_nava(const tabla_t *tabla,
								   const info_nava_t *info,
								   char orientare,
								   int x, int y)
{
	int lungime = info->lungime;
	int i;
	/* coordonate in afara tablei */
	if (x < 1 || x > tabla->n_lin || y < 1 || y > tabla->n_col) {
		return 0;
	}
	if (orientare == 'H') {
		if (y + lungime - 1 > tabla->n_col) {
			return 0;
		}
		for (i = 0; i < lungime; i++) {
			int poz = indice_tabla(tabla, x, y + i);
			if (tabla->harta_nave[poz] != -1) {
				return 0;
			}
		}
	} else if (orientare == 'V') {
		if (x - lungime + 1 < 1) {
			return 0;
		}
		for (i = 0; i < lungime; i++) {
			int poz = indice_tabla(tabla, x - i, y);
			if (tabla->harta_nave[poz] != -1) {
				return 0;
			}
		}
	} else {
		return 0;
	}
	return 1;
}

/* plasarea navei pe tabla */
static void plaseaza_nava(tabla_t *tabla,
						  const info_nava_t *info,
						  char orientare,
						  int x, int y)
{
	int id_nava = tabla->nr_nave;
	int i;
	nava_t *nava = &tabla->nave[id_nava];

	nava->tip_nava = info->id;
	nava->lungime = info->lungime;
	nava->orientare = orientare;
	nava->x_start = x;
	nava->y_start = y;
	nava->lovituri = 0;
	nava->este_distrusa = 0;

	if (orientare == 'H') {
		for (i = 0; i < nava->lungime; i++) {
			int poz = indice_tabla(tabla, x, y + i);
			tabla->harta_nave[poz] = id_nava;
		}
	} else {
		for (i = 0; i < nava->lungime; i++) {
			int poz = indice_tabla(tabla, x - i, y);
			tabla->harta_nave[poz] = id_nava;
		}
	}

	tabla->nr_nave++;
	tabla->nave_ramase++;
}

static void afisare_tabla(const tabla_t *tabla)
{
	int i,  j;
	for (i = 1; i <= tabla->n_lin; i++) {
		for (j = 1; j <= tabla->n_col; j++) {
			int poz = indice_tabla(tabla, i, j);
			int id_nava = tabla->harta_nave[poz];
			int valoare = 0;
			if (id_nava >= 0) {
				valoare = tabla->nave[id_nava].lungime;
			}
			if (j < tabla->n_col) {
				printf("%d ", valoare);
			} else {
				printf("%d", valoare);
			}
		}
		printf("\n");
	}
}

/* functia de atac */
static void ataca(tabla_t *tabla_tinta,
				  int id_atacator,
				  int x, int y,
				  statistici_t *stat)
{
	if (stat) {
		stat->lovituri_totale++;
	}
	if (x < 1 || x > tabla_tinta->n_lin ||
		y < 1 || y > tabla_tinta->n_col) {
		printf("Jucatorul %d a ratat o lovitura la coordonatele (%d, %d).\n",
			   id_atacator, x, y);
		return;
	}
	{
		int poz = indice_tabla(tabla_tinta, x, y);
		int id_nava;

		if (tabla_tinta->zone_atacate[poz]) {
			printf("Coordonatele (%d, %d) au fost deja atacate "
				   "de jucatorul %d.\n",
				   x, y, id_atacator);
			return;
		}
		tabla_tinta->zone_atacate[poz] = 1;
		id_nava = tabla_tinta->harta_nave[poz];

		if (id_nava == -1) {
			printf("Jucatorul %d a ratat o lovitura la "
				   "coordonatele (%d, %d).\n",
				   id_atacator, x, y);
			return;
		}
		nava_t *nava = &tabla_tinta->nave[id_nava];
		const info_nava_t *info;
		int este_distrusa = 0;

		if (nava->este_distrusa) {
			printf("Jucatorul %d a ratat o lovitura la "
				   "coordonatele (%d, %d).\n",
				   id_atacator, x, y);
			return;
		}

		if (stat) {
			stat->lovituri_nimerite++;
		}
		nava->lovituri++;
		info = info_tip(nava->tip_nava);

		if (x == nava->x_start && y == nava->y_start) {
			este_distrusa = 1;
		} else if (nava->lovituri == nava->lungime) {
			este_distrusa = 1;
		}
		if (este_distrusa) {
			int x_final, y_final;
			nava->este_distrusa = 1;
			tabla_tinta->nave_ramase--;

			if (nava->orientare == 'H') {
				x_final = nava->x_start;
				y_final = nava->y_start + nava->lungime - 1;
			} else {
				x_final = nava->x_start - nava->lungime + 1;
				y_final = nava->y_start;
			}
			printf("Jucatorul %d a distrus o nava %s plasata intre "
				   "coordonatele (%d, %d) si (%d, %d).\n",
				   id_atacator, info->nume,
				   nava->x_start, nava->y_start,
				   x_final, y_final);
		} else {
			printf("Jucatorul %d a lovit o nava %s la coordonatele (%d, %d).\n",
				   id_atacator, info->nume, x, y);
		}
	}
}

static void formateaza_rezultat(int precizie, long long rezultat, char *buffer)
{
	if (precizie < 0) {
		precizie = 0;
	}
	if (precizie > 10000) {
		precizie = 10000;
	}
	sprintf(buffer, "%03d.%02d.%lld",
			precizie / 100,
			precizie % 100,
			rezultat);
}

/* precizia unui jucator ca int in intervalul [0, 10000]*/
static int calculeaza_precizia_int(const statistici_t *stat)
{
	if (stat->lovituri_totale == 0) {
		return 0;
	}
	return (int)((stat->lovituri_nimerite * 10000LL) / stat->lovituri_totale);
}

/* numarul minim de lovituri neratate pentru a avea
	acuratetea mai mare sau egala cu X */
static long long cerinta_min_lovituri(const statistici_t *stat, int acuratete_X)
{
	long long H = stat->lovituri_nimerite;
	long long T = stat->lovituri_totale;
	long long acuratete_totala, acuratete_dorita, diferenta;
	if (T == 0) {
		return 0;
	}
	acuratete_totala = (long long)acuratete_X * T;
	acuratete_dorita = 10000LL * H;
	if (acuratete_dorita >= acuratete_totala) {
		return 0;
	}
	diferenta = acuratete_totala - acuratete_dorita;
	return (diferenta + 9999) / 10000;
}

/* numarul maxim de lovituri pe care le pot rata pentru a avea
	acuratetea mai mare sau egala cu X */
static long long cerinta_max_lovituri(const statistici_t *stat, int acuratete_X)
{
	long long H = stat->lovituri_nimerite;
	long long T = stat->lovituri_totale;
	long long acuratete_totala, acuratete_dorita, diferenta;
	if (T == 0) {
		return 0;
	}
	acuratete_totala = (long long)acuratete_X * T;
	acuratete_dorita = 10000LL * H;
	if (acuratete_dorita < acuratete_totala) {
		return 0;
	}
	diferenta = acuratete_dorita - acuratete_totala;
	return diferenta / 10000;
}

/* cerintele 1 si 4:
	daca este_min = 1 => numarul minim de meciuri cu 100% acuratete pentru
	a avea acuratetea totala mai mare sau egala cu X
	daca este_min = 0 => numarul maxim de lovituri pe care le pot rata pentru
	a avea acuratetea totala mai mare sau egala cu X
*/
static void sorteaza_indexuri(int *indexuri,
							  long double *modificari,
							  int limita,
							  int este_min)
{
	for (int i = 0; i < limita; i++) {
		for (int j = i + 1; j < limita; j++) {
			int ok = 0;
			if (este_min) {
				if (modificari[indexuri[j]] >
					modificari[indexuri[i]]) {
					ok = 1;
				}
			} else {
				if (modificari[indexuri[j]] <
					modificari[indexuri[i]]) {
					ok = 1;
				}
			}
			if (ok) {
				int aux = indexuri[i];
				indexuri[i] = indexuri[j];
				indexuri[j] = aux;
			}
		}
	}
}

static int calculeaza_numar_meciuri_min(int *indexuri,
										long double *modificari,
										int limita,
										long double suma_valori,
										long double suma_ponderi,
										long double acuratete_reala)
{
	long double medie_initiala = suma_valori / suma_ponderi;
	long double castig_cumulat = 0.0L;
	int k = 0;
	if (medie_initiala + EPS >= acuratete_reala) {
		return 0;
	}
	for (int j = 0; j < limita; j++) {
		int idx = indexuri[j];
		if (modificari[idx] <= 0.0L) {
			continue;
		}
		castig_cumulat += modificari[idx];
		k++;

		if ((suma_valori + castig_cumulat) / suma_ponderi + EPS >=
			 acuratete_reala) {
			return k;
		}
	}
	return k;
}

static int calculeaza_numar_meciuri_max(int *indexuri,
										long double *modificari,
										int limita,
										long double suma_valori,
										long double suma_ponderi,
										long double acuratete_reala)
{
	long double medie_initiala = suma_valori / suma_ponderi;
	long double pierdere_cumulata = 0.0L;
	int k = 0;
	if (medie_initiala + EPS < acuratete_reala) {
		return 0;
	}
	for (int j = 0; j < limita; j++) {
		int idx = indexuri[j];
		long double medie_daca_stricam;

		medie_daca_stricam =
			(suma_valori -
			 (pierdere_cumulata + modificari[idx])) /
			 suma_ponderi;
		if (medie_daca_stricam + EPS >= acuratete_reala) {
			pierdere_cumulata += modificari[idx];
			k++;
		} else {
			break;
		}
	}
	return k;
}

static void calculeaza_cerinte_1_si_4(statistici_t *stat,
									  int numar_meciuri,
									  int acuratete_X,
									  int este_min,
									  int *out_precizie,
									  int *out_numar_meciuri)
{
	long double suma_ponderi = 0.0L, suma_valori = 0.0L, modificari[256];
	long double ponderi[256];
	int limita, i, indexuri[256], k;
	if (numar_meciuri > 256) {
		limita = 256;
	} else {
		limita = numar_meciuri;
	}
	for (i = 0; i < limita; i++) {
		long double acc = 0.0L;
		indexuri[i] = i;
		ponderi[i] = (long double)stat[i].nave_per_meci;
		suma_ponderi += ponderi[i];

		if (stat[i].lovituri_totale > 0) {
			acc = (long double)stat[i].lovituri_nimerite /
				  (long double)stat[i].lovituri_totale;
		}
		suma_valori += acc * ponderi[i];
		if (este_min) {
			modificari[i] = (1.0L - acc) * ponderi[i];
		} else {
			modificari[i] = acc * ponderi[i];
		}
	}
	if (suma_ponderi == 0.0L) {
		*out_precizie = 0;
		*out_numar_meciuri = 0;
		return;
	}
	{
		long double medie_initiala = suma_valori / suma_ponderi;
		long double acuratete_reala = (long double)acuratete_X / 10000.0L;
		*out_precizie = (int)(medie_initiala * 10000.0L);
		*out_numar_meciuri = 0;
		sorteaza_indexuri(indexuri, modificari, limita, este_min);
		if (este_min) {
			k = calculeaza_numar_meciuri_min(indexuri, modificari,
											 limita, suma_valori,
											 suma_ponderi,
											 acuratete_reala);
		} else {
			k = calculeaza_numar_meciuri_max(indexuri, modificari,
											 limita, suma_valori,
											 suma_ponderi,
											 acuratete_reala);
		}
		*out_numar_meciuri = k;
	}
}

static int codifica_caracter(char c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'A' && c <= 'Z') {
		return 10 + (c - 'A');
	}
	if (c == '.') {
		return 36;
	}
	return 0;
}

static char decodifica_caracter(int valoare)
{
	if (valoare >= 0 && valoare <= 9) {
		return (char)('0' + valoare);
	}
	if (valoare >= 10 && valoare <= 35) {
		return (char)('A' + (valoare - 10));
	}
	if (valoare == 36) {
		return '.';
	}
	return '?';
}

static int mod37(int a)
{
	int r = a % DIM_ALFABET;
	if (r < 0) {
		r += DIM_ALFABET;
	}
	return r;
}

static int invers_modular(int a)
{
	int i;
	a = mod37(a);
	for (i = 1; i < DIM_ALFABET; i++) {
		if (mod37(a * i) == 1) {
			return i;
		}
	}
	return 1;
}

/* inversa unei matrici modulo 37 folosind Gauss-Jordan */
static void inversa_matrice_gauss(int n, int **mat, int **mat_inv)
{
	int i, j, k;
	int **a = (int **)malloc((size_t)n * sizeof(int *));
	for (i = 0; i < n; i++) {
		a[i] = (int *)malloc((size_t)(2 * n) * sizeof(int));
	}
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			a[i][j] = mod37(mat[i][j]);
		}
		for (j = n; j < 2 * n; j++) {
			if (i == (j - n)) {
				a[i][j] = 1;
			} else {
				a[i][j] = 0;
			}
		}
	}
	for (i = 0; i < n; i++) {
		int pivot = mod37(a[i][i]);
		int rand_pivot = i;
		if (pivot == 0) {
			for (k = i + 1; k < n; k++) {
				if (mod37(a[k][i]) != 0) {
					rand_pivot = k;
					pivot = mod37(a[k][i]);
					break;
				}
			}
			if (rand_pivot != i) {
				int *aux = a[i];
				a[i] = a[rand_pivot];
				a[rand_pivot] = aux;
			}
		}

		{
			int inv_pivot = invers_modular(pivot);
			for (j = 0; j < 2 * n; j++) {
				a[i][j] = mod37(a[i][j] * inv_pivot);
			}
		}
		for (k = 0; k < n; k++) {
			if (k != i) {
				int factor = mod37(a[k][i]);
				for (int j = 0; j < 2 * n; j++) {
					a[k][j] = mod37(a[k][j] - factor * a[i][j]);
				}
			}
		}
	}
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			mat_inv[i][j] = a[i][j + n];
		}
	}
	for (i = 0; i < n; i++) {
		free(a[i]);
	}
	free(a);
}

/* codificarea/decodificarea mesajului */
static void transformare_mesaj_hill(const char *cheie,
									char *mesaj, int este_decriptare)
{
	int lungime_mesaj, lungime_cheie, dim_maxima;
	char *buffer;
	int **mat, **mat_operatie;
	int poz_out = 0, i, j;
	if (strcmp(cheie, "-") == 0) {
		return;
	}
	lungime_mesaj = (int)strlen(mesaj);
	lungime_cheie = (int)strlen(cheie);
	dim_maxima = (int)sqrt((double)lungime_cheie);
	if (dim_maxima < 1) {
		dim_maxima = 1;
	}
	buffer = (char *)malloc((size_t)(lungime_mesaj + 1) * sizeof(char));
	if (!buffer) {
		return;
	}
	mat = (int **)malloc((size_t)dim_maxima * sizeof(int *));
	mat_operatie = (int **)malloc((size_t)dim_maxima * sizeof(int *));
	for (int i = 0; i < dim_maxima; i++) {
		mat[i] = (int *)malloc((size_t)dim_maxima * sizeof(int));
		mat_operatie[i] = (int *)malloc((size_t)dim_maxima * sizeof(int));
	}
	{
		int k = 0;
		while (k < lungime_mesaj) {
			int ramase = lungime_mesaj - k;
			int dim_bloc;
			int vector_intrare[100];
			int vector_iesire[100];
			if (ramase < dim_maxima) {
				dim_bloc = ramase;
			} else {
				dim_bloc = dim_maxima;
			}
			for (i = 0; i < dim_bloc; i++) {
				for (j = 0; j < dim_bloc; j++) {
					int index_cheie = i * dim_bloc + j;
					mat[i][j] = codifica_caracter(cheie[index_cheie]);
				}
			}
			if (este_decriptare) {
				inversa_matrice_gauss(dim_bloc, mat, mat_operatie);
			} else {
				for (i = 0; i < dim_bloc; i++) {
					for (j = 0; j < dim_bloc; j++) {
						mat_operatie[i][j] = mat[i][j];
					}
				}
			}
			for (i = 0; i < dim_bloc; i++) {
				vector_intrare[i] = codifica_caracter(mesaj[k + i]);
			}
			for (i = 0; i < dim_bloc; i++) {
				int sum = 0;
				for (int j = 0; j < dim_bloc; j++) {
					sum += mat_operatie[i][j] * vector_intrare[j];
					sum = mod37(sum);
				}
				vector_iesire[i] = sum;
			}
			for (i = 0; i < dim_bloc; i++) {
				buffer[poz_out] = decodifica_caracter(vector_iesire[i]);
				poz_out++;
			}
			k += dim_bloc;
		}
	}
	buffer[poz_out] = '\0';
	strcpy(mesaj, buffer);
	for (i = 0; i < dim_maxima; i++) {
		free(mat[i]);
		free(mat_operatie[i]);
	}
	free(mat);
	free(mat_operatie);
	free(buffer);
}

static char *citeste_cuv_dinamic(FILE *f)
{
	size_t dim = 64;
	char *sir = (char *)malloc(dim * sizeof(char));
	size_t lungime = 0;
	int c;

	if (!sir) {
		return NULL;
	}
	/* sar peste spatii si newline */
	do {
		c = fgetc(f);
		if (c == EOF) {
			free(sir);
			return NULL;
		}
	} while (isspace(c));
	sir[lungime] = (char)c;
	lungime++;
	while (1) {
		c = fgetc(f);
		if (c == EOF || isspace(c)) {
			break;
		}
		if (lungime + 1 >= dim) {
			char *sir_nou;
			dim *= 2;
			sir_nou = (char *)realloc(sir, dim * sizeof(char));
			if (!sir_nou) {
				free(sir);
				return NULL;
			}
			sir = sir_nou;
		}
		sir[lungime] = (char)c;
		lungime++;
	}
	sir[lungime] = '\0';
	return sir;
}

static void faza_atac(tabla_t *t1, tabla_t *t2,
					  statistici_t **statistici_jucator,
					  int indice_meci_curent)
{
	int atacator = 0, end_game = 0;
	while (!end_game) {
		int x, y;
		tabla_t *tabla_tinta;
		statistici_t *stat_curente;
		if (scanf("%d %d", &x, &y) != 2) {
			break;
		}
		if (atacator == 0) {
			tabla_tinta = t2;
			stat_curente = &statistici_jucator[0][indice_meci_curent];
		} else {
			tabla_tinta = t1;
			stat_curente = &statistici_jucator[1][indice_meci_curent];
		}
		ataca(tabla_tinta, atacator + 1, x, y, stat_curente);
		if (tabla_tinta->nave_ramase == 0) {
			printf("Jucatorul %d a castigat.\n", atacator + 1);
			end_game = 1;
		}
		atacator = 1 - atacator;
	}
}

static void plaseaza_nave(tabla_t *t1, tabla_t *t2,
						  int limite_jucator[2][NUMAR_TIPURI_NAVA],
						  int nave_totale_per_jucator)
{
	int  nave_puse[2] = { 0, 0 }, tura = 0;
	while (nave_puse[0] < nave_totale_per_jucator ||
		   nave_puse[1] < nave_totale_per_jucator) {
		char tip_nava, orientare;
		int x, y, index_tip, ok = 0;
		tabla_t *tabla_curenta;
		const info_nava_t *info;
		if (scanf(" %c %c %d %d",
				  &tip_nava, &orientare, &x, &y) != 4) {
			break;
		}
		if (tura == 0) {
			tabla_curenta = t1;
		} else {
			tabla_curenta = t2;
		}
		info = info_tip(tip_nava);
		index_tip = pozitie_tip_nava(tip_nava);
		if (!info || index_tip == -1) {
			ok = 1;
		} else if (limite_jucator[tura][index_tip] == 0) {
			ok = 1;
		} else if (!verificare_plasare_nava(tabla_curenta,
											info,
											orientare,
											x, y)) {
			ok = 1;
		}
		if (ok) {
			printf("Nava %s nu poate fi amplasata %s la "
				   "coordonatele (%d, %d).\n",
				   (info ? info->nume : "Unknown"),
				   (orientare == 'H' ? "orizontal" : "vertical"),
				   x, y);
		} else {
			plaseaza_nava(tabla_curenta, info, orientare, x, y);
			limite_jucator[tura][index_tip]--;
			nave_puse[tura]++;
			tura = 1 - tura;
		}
	}
}

static void init_meci(int *n_lin, int *n_col, tabla_t *t1, tabla_t *t2)
{
	if (scanf("%d %d", n_lin, n_col) != 2) {
		return;
	}
	initializare_tabla(t1, *n_lin, *n_col);
	initializare_tabla(t2, *n_lin, *n_col);
}

static void proceseaza_coduri(statistici_t **statistici_jucator,
							  int numar_total_meciuri)
{
	char *cheie = citeste_cuv_dinamic(stdin);
	if (cheie) {
		char *cod;
		while (1) {
			cod = citeste_cuv_dinamic(stdin);
			if (!cod) {
				break;
			}
			if (strcmp(cod, "Q") == 0) {
				free(cod);
				break;
			}
			transformare_mesaj_hill(cheie, cod, 1);
			int index_jucator, index_meci, acuratete_X, precizie_finala = 0;
			char tip_cerinta1, tip_cerinta2[4], rezultat[512];
			if (cod[0] == 'O') {
				index_jucator = 0;
			} else {
				index_jucator = 1;
			}
			tip_cerinta1 = cod[1];
			index_meci = (cod[2] - '0') * 100 +
						 (cod[3] - '0') * 10 +
						 (cod[4] - '0');
			acuratete_X = (cod[5] - '0') * 10000 +
						  (cod[6] - '0') * 1000 +
						  (cod[7] - '0') * 100 +
						  (cod[9] - '0') * 10 +
						  (cod[10] - '0');
			strncpy(tip_cerinta2, cod + 11, 3);
			tip_cerinta2[3] = '\0';
			if (tip_cerinta1 == 'U') {
				long long raspuns = 0;
				statistici_t *stat_meci;
				if (index_meci < 1 ||
					index_meci > numar_total_meciuri) {
					printf("Nu exista date pentru meciul cerut.\n");
					free(cod);
					continue;
				}
				stat_meci = &statistici_jucator[index_jucator][index_meci - 1];
				precizie_finala = calculeaza_precizia_int(stat_meci);
				if (strcmp(tip_cerinta2, "MIN") == 0) {
					raspuns = cerinta_min_lovituri(stat_meci, acuratete_X);
				} else {
					raspuns = cerinta_max_lovituri(stat_meci, acuratete_X);
				}
				formateaza_rezultat(precizie_finala,
									raspuns,
									rezultat);
			} else {
				int raspuns_int = 0;
				if (strcmp(tip_cerinta2, "MIN") == 0) {
					calculeaza_cerinte_1_si_4(statistici_jucator[index_jucator],
											  numar_total_meciuri, acuratete_X,
											  1, &precizie_finala,
											  &raspuns_int);
				} else {
					calculeaza_cerinte_1_si_4(statistici_jucator[index_jucator],
											  numar_total_meciuri, acuratete_X,
											  0, &precizie_finala,
											  &raspuns_int);
				}
				formateaza_rezultat(precizie_finala, (long long)raspuns_int,
									rezultat);
			}
			transformare_mesaj_hill(cheie, rezultat, 0);
			printf("%s\n", rezultat);
			free(cod);
		}
		free(cheie);
	}
}

int main(void)
{
	int numar_meciuri, numar_total_meciuri, indice_meci_curent = 0;
	statistici_t *statistici_jucator[2];
	if (scanf("%d", &numar_meciuri) != 1) {
		return 0;
	}
	statistici_jucator[0] =
		(statistici_t *)calloc((size_t)numar_meciuri, sizeof(statistici_t));
	statistici_jucator[1] =
		(statistici_t *)calloc((size_t)numar_meciuri, sizeof(statistici_t));
	numar_total_meciuri = numar_meciuri;
	while (numar_meciuri > 0) {
		int n_lin, n_col, i;
		tabla_t tabla_jucator1, tabla_jucator2;
		int limite[NUMAR_TIPURI_NAVA], limite_jucator[2][NUMAR_TIPURI_NAVA];
		int nave_totale_per_jucator = 0;
		init_meci(&n_lin, &n_col, &tabla_jucator1, &tabla_jucator2);
		limite[0] = (n_lin * n_col) / 70;
		limite[1] = (n_lin * n_col) / 55;
		limite[2] = (n_lin * n_col) / 40;
		limite[3] = (n_lin * n_col) / 30;
		limite[4] = (n_lin * n_col) / 20;
		for (i = 0; i < NUMAR_TIPURI_NAVA; i++) {
			limite_jucator[0][i] = limite[i];
			limite_jucator[1][i] = limite[i];
			nave_totale_per_jucator += limite[i];
		}
		plaseaza_nave(&tabla_jucator1, &tabla_jucator2,
					  limite_jucator, nave_totale_per_jucator);
		statistici_jucator[0][indice_meci_curent].nave_per_meci =
			nave_totale_per_jucator;
		statistici_jucator[1][indice_meci_curent].nave_per_meci =
			nave_totale_per_jucator;
		afisare_tabla(&tabla_jucator1);
		printf("\n");
		afisare_tabla(&tabla_jucator2);
		faza_atac(&tabla_jucator1, &tabla_jucator2,
				  statistici_jucator, indice_meci_curent);
		elibereaza_tabla(&tabla_jucator1);
		elibereaza_tabla(&tabla_jucator2);
		numar_meciuri--; indice_meci_curent++;
	}
	proceseaza_coduri(statistici_jucator, numar_total_meciuri);
	free(statistici_jucator[0]);
	free(statistici_jucator[1]);
	return 0;
}
