/*

Algoritmo de Self-Organizing Map por Teuvo Kohonen (1984)
Implementado por Geraldo Rabelo geraldo.rabelo@gmail.com em 24/08/2017


Como SOM funciona?

        As entradas (input.dat) representam um conjunto de padrões a ser reconhecido pela rede após seu treino.
        
        Treino Passo a Passo:
        
                0 - Iniciar todos os neuronios da rede com pesos aleatórios;
                1 - Pegar uma entrada por vez;
                2 - Verificar cada neuronio da rede em busca daquele que mais se aproxime da entrada selecionada no passo 1;
                3 - Após encontrar um neuronio vencedor, contaminar sua vizinhança fazendo-a tender à sua cor;
                4 - voltar ao passo 1
                
                Obs.: 
                        O parametro TAXA DE APRENDIZADO garante que apenas uma fração da distancia entre as cores do neuronio selecionado e o neuronio vencedor será aplicada à vizinhança deste, em menor grau a pedida que aumenta sua distancia posicional.
                        
                        O parametro RAIO deve iniciar com valor alto (largura da base) e ser gradualmente decrementado (até 1).
                        
                        O parametro EPOCAS deve ser escolhido de acordo com a area da rede.

        Uso:
                Após o treino, a rede deverá ser capaz de sinalizar se uma dada entrada possui ou não classe correspondente.                
                
Compilando:

        gcc -o KN2D KN2D.c -lm

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define CONFIG		"config.dat"
#define INPUT		"input.dat"
#define OUTPUT		"output.dat"
#define OUTPUT_HTML	"output.html"
#define START		"start.dat"
#define START_HTML      "start.html"
#define STYLE           "styles.css"

struct neuronio {    
    int         *pesos;
    int         *posicao;
    float       saida;
} dot;
typedef struct neuronio Neuronio;

struct entrada {
    int         *pesos;
} in;
typedef struct entrada Entrada;

int     winnerWeightDistance;
int	winnerIndex;

float   distanciaEuclidiana     = 0.0;
float   LEARNING_RATE           = 0;
int     NEIGHBOURHOOD_RADIUS    = 0;
int     NUMBER_OF_INPUTS        = 0;
int     NUMBER_OF_NEURONS       = 0;
int     INPUT_DIMENSION         = 0;
int     EPOCHS                  = 0;
int     COORDINATES_DIMENSION   = 0;
int     epoch                   = 0;
int     distancia_do_peso               = 0;
int     neuronios_por_linha     = 0;

Entrada  *entradas              = NULL;
Neuronio *neuronios             = NULL;

int testa_se_arquivo_existe(char *nome_do_arquivo) {

        int result = 1;
        FILE *arquivo = NULL;
        
        arquivo = fopen(nome_do_arquivo,"r");
        
        if (arquivo == NULL) {
                result = 0;
        }
        
        return result;
}

void load_cfg_from_file(FILE *arquivo)
{
		
        int linhaContador = 0;
        char valor[8];
        char linha[32];

        memset(linha,'\0',32);
        memset(valor,'\0',8);

	rewind(arquivo);

	while (!feof(arquivo))
	{
		fgets(linha,32,arquivo);
	        for (int contador = 0; contador < strlen(linha); contador++)
                {
                        if (linha[contador] == ':')
                        {
                                for (int contadorValorCaracteres = contador+1; contadorValorCaracteres < strlen(linha); contadorValorCaracteres++)
                                {
                                        valor[contadorValorCaracteres-contador-1] = linha[contadorValorCaracteres];
                                }

                                switch(linhaContador) {
                                        case 0: 
                                                EPOCHS = atoi(valor);
                                                break;
                                        case 1:
                                                LEARNING_RATE = atof(valor);
                                                break;
                                        case 2:
                                                NEIGHBOURHOOD_RADIUS = atoi(valor);
                                                break;
                                        case 3:
                                                NUMBER_OF_INPUTS = atoi(valor);
                                                break;
                                        case 4:
                                                INPUT_DIMENSION = atoi(valor);
                                                break;
                                        case 5:
                                                NUMBER_OF_NEURONS = atoi(valor);
                                                break;
                                        case 6:
                                                COORDINATES_DIMENSION = atoi(valor);
                                                break;

                                }
                        }
                }		
                memset(linha,'\0',32);
                memset(valor,'\0',8);
		linhaContador++;
	}
}

int getLinhasArquivo(FILE *arquivo)
{
	int resultado = 0;
	char caractere;
	
	rewind(arquivo);
	while (!feof(arquivo))
	{
		caractere = fgetc(arquivo);
		if(caractere == '\n')
		{
		  resultado++;
		}
	}
	return resultado;
}

char *substring (char *linha, int inicio, int fim)
{
	char *resultado = (char *) malloc(strlen(linha)*sizeof(char));

	for (int contador = inicio; contador<fim; contador++)
	{
		resultado[contador-inicio] = linha[contador];

	}
	
	return resultado;
} 

void load_inputs(FILE *file_input) {

        int linhaContador = 0;
        int entradaContador = 0;
        int inicioValor = 0;
        int valor = 0;
        char linha[14];

        entradas = malloc(NUMBER_OF_INPUTS*sizeof(Entrada));
        for (int c0 = 0; c0 < NUMBER_OF_INPUTS; c0++)
        {
                entradas[c0].pesos = (int *) malloc(INPUT_DIMENSION*sizeof(int));  
        }
        
	rewind(file_input);        
	while (!feof(file_input))
	{
                memset(linha,'\0',14);
		fgets(linha,14,file_input);

		entradaContador = 0;
		inicioValor = 0;
                //printf("\nLinha %d; %d caracteres; %s",linhaContador,strlen(linha),linha);

	        for (int contador = 0; contador < strlen(linha)-1; contador++)
                {
                        if (linha[contador] == ';')
                        {                                
                                valor = atoi(substring(linha,inicioValor,contador));
                                entradas[linhaContador].pesos[entradaContador] = valor;  

                                inicioValor = contador+1;
                                entradaContador++;
                        } 
                }
                linhaContador++;
                if (linhaContador == NUMBER_OF_INPUTS) break;                
        }        
}

void load_start(FILE *file_start) {

        int linhaContador       = 0;
        int entradaContador     = 0;
        int inicioValor         = 0;
        int neuronio_coluna     = 0;
        int neuronio_linha      = 0;
        
        int neuronios_por_linha = sqrt(NUMBER_OF_NEURONS);
        
        int valor               = 0;
        char linha[14];
        
	rewind(file_start);        
	while (!feof(file_start))
	{
                memset(linha,'\0',14);
		fgets(linha,14,file_start);

		entradaContador = 0;    //      de 1 a 3
		inicioValor = 0;
		
                if (neuronio_linha == neuronios_por_linha)
                {
                        neuronio_linha = 0;
                        neuronio_coluna++;
                }
                                        
                neuronios[linhaContador].posicao[0] = neuronio_linha;
                neuronios[linhaContador].posicao[1] = neuronio_coluna;
                                        
                neuronio_linha++;

                //printf("\n%s",linha);
	        for (int contador = 0; contador < strlen(linha); contador++)
                {
                        if (linha[contador] == ';')
                        {                                
                                valor = atoi(substring(linha,inicioValor,contador));
                                //printf("\n%d valor: %d; linha: %d; coluna: %d, %s",linhaContador,valor,neuronio_linha-1,neuronio_coluna,linha);

                                neuronios[linhaContador].pesos[entradaContador] = valor;                                          
                                        
                                inicioValor = contador+1;
                                entradaContador++;
                        } 
                }
              
                linhaContador++;
                if (linhaContador == NUMBER_OF_NEURONS) break;
        } 
}

void criar_rede_em_arquivo_dat(FILE *file_start) {

        int neuronios_por_linha = sqrt(NUMBER_OF_NEURONS);
        int neuronios_por_coluna = neuronios_por_linha;
        
        int contador_linha = 0;
        int contador_coluna = 0;
        
        neuronios = malloc(NUMBER_OF_NEURONS*sizeof(Neuronio));
        
        for (int c=0; c < NUMBER_OF_NEURONS; c++) {
                neuronios[c].pesos = (int *) malloc(INPUT_DIMENSION*sizeof(int));
                for (int c0 = 0;c0 < INPUT_DIMENSION;c0++) {
/*
        Inicializando pesos
*/
                                neuronios[c].pesos[c0] = (rand() % 255);
                                fprintf(file_start,"%d;",neuronios[c].pesos[c0]);
                }
                fprintf(file_start,"\n");
                neuronios[c].posicao = (int *) malloc(COORDINATES_DIMENSION*sizeof(int));
                                                
                neuronios[c].posicao[0] = contador_linha;
                neuronios[c].posicao[1] = contador_coluna;
                        
                                
                contador_linha++;

                if (contador_linha == neuronios_por_linha) {
                        contador_coluna++;
                        contador_linha = 0;
                }
        }
}

void criar_arquivo_css() {
        FILE *file_css = NULL;
        
        file_css = fopen(STYLE,"w+");
        
        if (file_css == NULL) {
                printf("\nSTYLE.CSS not found.");
        } else {
                fprintf(file_css,"table,tr,td {\n\tborder: none;\n\tborder-collapse: none;\n\tborder-spacing: 0px;\n}\n\n.cell {\n\tfont-style: normal;\n\tfont-size: 5px;\n\tfont-weight: 700;\n\tfont-family: Helvetica, Arial, Sans-serif;\n\ttext-align: center;\n\tmargin: 0;\n\tpadding: 0;\n}");
        }
}

void imprimir_rede_em_arquivo_html(FILE *arquivo)
{
        int neuronios_por_linha = sqrt(NUMBER_OF_NEURONS);

        fprintf(arquivo,"<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\" /></head><body><table class=\"cell\"><tr>");
        for (int c0=0; c0 < NUMBER_OF_NEURONS; c0++) {

                if (c0 % neuronios_por_linha == 0 && c0 > 0)
                {
                        fprintf(arquivo,"</td></tr><tr><td>");
                } else {
                        fprintf(arquivo,"<td>");
                }
                
                fprintf(arquivo,"<div style=\"background-color: rgb(");
                for (int c1=0; c1 < INPUT_DIMENSION; c1++) {
                     if (c1 == INPUT_DIMENSION-1)
                     {
                        fprintf(arquivo,"%d);",neuronios[c0].pesos[c1]);
                     } else {
                             fprintf(arquivo,"%d,",neuronios[c0].pesos[c1]);
                     }
                }                
                fprintf(arquivo,"\">&nbsp;&nbsp;");

/*
                fprintf(arquivo,"%d<br>(%d,%d)<br>(",c0,neuronios[c0].posicao[0],neuronios[c0].posicao[1]);
                for (int c1=0; c1 < INPUT_DIMENSION; c1++) {
                     if (c1 == INPUT_DIMENSION-1)
                     {
                        fprintf(arquivo,"%d);",neuronios[c0].pesos[c1]);
                     } else {
                             fprintf(arquivo,"%d,",neuronios[c0].pesos[c1]);
                     }
                }
*/

//                fprintf(arquivo,"&nbsp;");
                fprintf(arquivo,"</div></td>");
                                
        }
        fprintf(arquivo,"</table></body></html>");
}

void imprimir_rede_na_tela() {
        printf("\n\n");
        printf("numero de neuronios: %d\ndimensao: %d",NUMBER_OF_NEURONS, INPUT_DIMENSION);
        for (int c0=0; c0 < NUMBER_OF_NEURONS; c0++) {
                printf("\nNeuronio %d",c0);
                for (int c1=0; c1 < INPUT_DIMENSION; c1++) {
                        printf("\n\tpeso %d: ",c1);
                        //for (int c2; c2 < INPUT_DIMENSION; c2++) {
                                printf("%d ",neuronios[c0].pesos[c1]);
                        //}                        
                }                
                for (int c2=0; c2 < COORDINATES_DIMENSION; c2++) {
                        printf("\n\tposicao %d: ",c2);
                        //for (int c2; c2 < INPUT_DIMENSION; c2++) {
                                printf("%d ",neuronios[c0].posicao[c2]);
                        //}                        
                }                
                
        }
        printf("\n\n");        
}

Neuronio get_neuronio_por_posicao(int x, int y)
{
        Neuronio result;
        
        for (int c2 = 0; c2 < NUMBER_OF_NEURONS; c2++)
        {       
                if(neuronios[c2].posicao[0] == x && neuronios[c2].posicao[1] == y)
                {
                        result = neuronios[c2];                        
                }
        }
        
        return result;
}

float decremento(int valorInicial)
{

        //      decremento para taxa de aprendizado e raio da vizinhança        
        return 1+valorInicial*exp(-(epoch/(EPOCHS/10)));
}

float distancia_do_vencedor(Neuronio neuronioAtual)
{
        float result = sqrt(pow((neuronioAtual.posicao[0]-neuronios[winnerIndex].posicao[0]),2)+pow((neuronioAtual.posicao[1]-neuronios[winnerIndex].posicao[1]),2));
        //printf("\ndistancia do vencedor: %f",result);
        return result;
}

float bonus_vizinhanca(Neuronio neuronioAtual)
{
        distanciaEuclidiana = distancia_do_vencedor(neuronioAtual);
        float result = 0.0;

	if (distanciaEuclidiana > 0)
	{
		result = exp(-5*pow(distanciaEuclidiana,2)/decremento(NEIGHBOURHOOD_RADIUS));
//		result = exp(-distanciaEuclidiana/decremento(NEIGHBOURHOOD_RADIUS));
	}

        return result;
}


int WeightDistance(Entrada entrada, Neuronio neuronio) {
        int resultado = 0;
        
        for (int c0 = 0; c0 < INPUT_DIMENSION; c0++) { // nao é NUMBER_OF_INPUTS; corrigido!
                resultado = resultado + pow(entrada.pesos[c0]-neuronio.pesos[c0],2);
        }
        
        return sqrt(resultado);
}

void ajuste()
{
        
        for(int c0=0; c0 < NUMBER_OF_NEURONS; c0++)
        {
                if (c0 != winnerIndex) {
                        for (int c1=0; c1 < INPUT_DIMENSION;c1++)
                        {
                                //no caso das cores, vamos somar ou subtrair?
                                neuronios[c0].pesos[c1] = neuronios[c0].pesos[c1] - LEARNING_RATE*bonus_vizinhanca(neuronios[c0])*(neuronios[c0].pesos[c1]-neuronios[winnerIndex].pesos[c1]);
                        }
                }
                
        }
}

void kohonen() {

        distancia_do_peso = 0;
        neuronios_por_linha = sqrt(NUMBER_OF_NEURONS);
        
        for(int c0 = 0; c0 < NUMBER_OF_INPUTS;c0++) {
               /*
                         pegar entrada
                */

                for(int c1 = 0;c1 < NUMBER_OF_NEURONS; c1++) {
                
                        distancia_do_peso = WeightDistance(entradas[c0],neuronios[c1]);
                        
                        if (distancia_do_peso == 0) {
                                winnerWeightDistance = distancia_do_peso;
                                winnerIndex = c1;                               

                                ajuste();
                        } else if (distancia_do_peso <= winnerWeightDistance) // o menor-igual aqui faz toda a diferenca! senao usar, nao expande outras areas
                        {
                                winnerWeightDistance = distancia_do_peso;
                                winnerIndex = c1;                               
                        }
                }
                ajuste();
                winnerWeightDistance = 999999999;
        }
        

}

int total_de_entradas() {
        int c = 0;
        Entrada *entradaAtual = entradas;
        
        while (entradaAtual[c].pesos) {
                c++;
        }
        
        return c;
}

void main() {
        
        FILE *file_cfg          = NULL;
        FILE *file_input        = NULL;
        FILE *file_start        = NULL;
        FILE *file_output       = NULL;                        
        FILE *file_start_html   = NULL;
        FILE *file_output_html  = NULL;                        
                

	file_cfg                = fopen(CONFIG,"r");
	file_input              = fopen(INPUT,"r");	
	file_output             = fopen(OUTPUT,"w+");
	file_output_html        = fopen(OUTPUT_HTML,"w+");
	file_start_html         = fopen(START_HTML,"w+");			

        if (testa_se_arquivo_existe(START) == 0) {
                file_start      = fopen(START,"w+");
        } else {
                file_start      = fopen(START,"r");
        }
	
	if (file_cfg == NULL)
	{
	        printf("\nCFG file not found.\n");
		exit(1);
	}	

	if (file_input == NULL)
	{
	        printf("\nINPUT file not found.\n");
		exit(1);
	}	

	if (file_start == NULL)
	{
	        printf("\nSTART file not found.\n");
		exit(1);
	}	

	if (file_output == NULL)
	{
	        printf("\nOUTPUT file not found.\n");
		exit(1);
	}	

	if (file_output_html == NULL)
	{
	        printf("\nOUTPUT_HTML file not found.\n");
		exit(1);
	}	
        
	if (file_start_html == NULL)
	{
	        printf("\nSTART_HTML file not found.\n");
		exit(1);
	}	

	load_cfg_from_file(file_cfg);

        criar_rede_em_arquivo_dat(file_start);

        winnerWeightDistance = 99999999;
        
        load_inputs(file_input);
        load_start(file_start);
        
        epoch = 0;
        
        //imprimir_rede_na_tela();                

        imprimir_rede_em_arquivo_html(file_start_html);

        while (epoch < EPOCHS)
        {
                kohonen();
                epoch++;
        }
        
        imprimir_rede_em_arquivo_html(file_output_html);
        
        if (testa_se_arquivo_existe(STYLE) == 0) {
                criar_arquivo_css();
        }
                
//        printf("\ntotal de entradas: %d",total_de_entradas());
        
        fclose(file_cfg);
        fclose(file_input);        
        fclose(file_start);
        fclose(file_output);        
        fclose(file_output_html);
        fclose(file_start_html);        
}
