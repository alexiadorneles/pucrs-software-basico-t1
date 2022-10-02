#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // Para usar strings
#include <stdbool.h>

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

// Um pixel Pixel (24 bits)
typedef struct
{
    unsigned char r, g, b;
} Pixel;

// Uma imagem Pixel
typedef struct
{
    int width, height;
    Pixel *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void processa();

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Largura e altura da janela
int width, height;

// Métodos próprios
Pixel **criarMatriz(Pixel (*in)[width], int i, int j);

// Fator de multiplicação do ruído
int fator;

// Identificadores de textura
GLuint tex[2];

// As 2 imagens
Img pic[2];

// Imagem selecionada (0,1)
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (Pixel *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

int main(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("seeing [im. entrada]\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem de entrada
    // pic[1] -> imagem de saida

    // Carrega a imagem
    load(argv[1], &pic[0]);

    fator = 5;
    if (argc == 3)
        fator = atoi(argv[2]);

    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem de entrada (0)
    pic[1].width = pic[0].width;
    pic[1].height = pic[0].height;
    pic[1].img = calloc(pic[1].width * pic[1].height, 3); // W x H x 3 bytes (Pixel)

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Analise Forense de Imagens");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Exibe as dimensões na tela, para conferência
    printf("Entrada  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    sel = 0; // entrada

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Aplica o algoritmo
    processa();

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

Pixel **criarMatriz(Pixel (*in)[width], int i, int j)
{
    Pixel matriz[3][3] = {};
    int iNorte = i - 1;
    int jNorte = j;

    int iLeste = i;
    int jLeste = j + 1;

    int iSul = i + 1;
    int jSul = j;

    int iOeste = i;
    int jOeste = j - 1;

    int iNordeste = i - 1;
    int jNordeste = j + 1;

    int iSudeste = i + 1;
    int jSudeste = j + 1;

    int iNoroeste = i - 1;
    int jNoroeste = j - 1;

    int iSudoeste = i + 1;
    int jSudoeste = j - 1;

    // cantos
    bool cantoSuperiorEsquerdo = i == 0 && j == 0;
    bool cantoSuperiorDireito = i == 0 && j == width - 1;
    bool cantoInferiorEsquerdo = i == width - 1 && j == 0;
    bool cantoInferiorDireito = i == width - 1 && j == width - 1;

    if (cantoSuperiorEsquerdo || cantoInferiorDireito)
    {
        Pixel sudeste = in[iSudeste][jSudeste];
        // sudoeste
        matriz[2][0] = sudeste;
        // nordeste
        matriz[0][2] = sudeste;
    }

    if (cantoSuperiorDireito || cantoInferiorEsquerdo)
    {
        Pixel sudoeste = in[iSudoeste][jSudoeste];
        // noroeste
        matriz[0][0] = sudoeste;
        // sudeste
        matriz[2][2] = sudoeste;
    }

    matriz[1][1] = in[i][j];

    bool norteEhValido = iNorte >= 0 && iNorte <= width && jNorte >= 0 && jNorte <= height;
    matriz[0][1] = norteEhValido ? in[iNorte][jNorte] : in[iSul][jSul];
    bool sulEhValido = iSul >= 0 && iSul <= width && jSul >= 0 && jSul <= height;
    matriz[2][1] = sulEhValido ? in[iSul][jSul] : in[iNorte][jNorte];

    bool lesteEhValido = iLeste >= 0 && iLeste <= width && jLeste >= 0 && jLeste <= height;
    matriz[1][2] = lesteEhValido ? in[iLeste][jLeste] : in[iOeste][jOeste];
    bool oesteEhValido = iOeste >= 0 && iOeste <= width && jOeste >= 0 && jOeste <= height;
    matriz[1][0] = oesteEhValido ? in[iOeste][jOeste] : in[iLeste][jLeste];

    if (!cantoSuperiorEsquerdo && !cantoInferiorDireito)
    {

        bool nordesteEhValido = iNordeste >= 0 && iNordeste <= width && jNordeste >= 0 && jNordeste <= height;
        matriz[0][2] = nordesteEhValido ? in[iNordeste][jNordeste] : in[iSudoeste][jSudoeste];
        bool sudoesteEhValido = iSudoeste >= 0 && iSudoeste <= width && jSudoeste >= 0 && jSudoeste <= height;
        matriz[2][0] = sudoesteEhValido ? in[iSudoeste][jSudoeste] : in[iNordeste][jNordeste];
    }

    if (!cantoSuperiorDireito && !cantoInferiorEsquerdo)
    {
        bool noroesteEhValido = iNoroeste >= 0 && iNoroeste <= width && jNoroeste >= 0 && jNoroeste <= height;
        matriz[0][0] = noroesteEhValido ? in[iNoroeste][jNoroeste] : in[iSudeste][jSudeste];
        bool sudesteEhValido = iSudeste >= 0 && iSudeste <= width && jSudeste >= 0 && jSudeste <= height;
        matriz[2][2] = sudesteEhValido ? in[iSudeste][jSudeste] : in[iNoroeste][jNoroeste];
    }

    return matriz;
}

// Aplica o algoritmo e gera a saída em pic[1]
void processa()
{
    // Converte para interpretar como matriz
    Pixel(*in)[width] = (Pixel(*)[width])pic[0].img;
    Pixel(*out)[width] = (Pixel(*)[width])pic[1].img;

    // Aplica o algoritmo e gera a saida em out (pic[1].img)
    // ...
    // ...
    int regsize = 3;
    int tam = regsize * regsize;

    printf("Region size*size: %d\n", tam);
    printf("Fator: %d\n\n", fator);

    // Exemplo: inverte as cores da imagem de entrada
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {

            Pixel **matriz = criarMatriz(in, i, j);

            out[i][j].r = 255 - in[i][j].r;
            out[i][j].g = 255 - in[i][j].g;
            out[i][j].b = 255 - in[i][j].b;

            // criar o array imutavel com os pixeis (com r, g, b)
            // cria struct da luminancia com id
            // cria um arrau com essas structs
            // ordena o array de cima pela luminancia
            // pega mediana
            // pega a cor da mediana a partir do id dentro do array imutavel
            // pixelOriginal.r = pixelOriginal.r - mediana.r < 0 ? 0 : pixelOriginal.r - mediana.r
            // pixelOriginal.g = pixelOriginal.g - mediana.g < 0 ? 0 : pixelOriginal.g - mediana.g
            // pixelOriginal.b = pixelOriginal.b - mediana.b < 0 ? 0 : pixelOriginal.b - mediana.b
            // out[i][j] = pixelOriginal
        }
    }

    // Se desejar salvar a imagem de saída, descomente a linha abaixo
    // SOIL_save_image("out.bmp", SOIL_SAVE_TYPE_BMP, pic[1].width, pic[1].height, 3, (const unsigned char *)pic[1].img);

    // Faz upload da nova textura na GPU - NÃO ALTERAR
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, out);
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        free(pic[0].img);
        free(pic[1].img);
        exit(1);
    }
    if (key >= '1' && key <= '2')
        // 1-2: seleciona a imagem correspondente (origem ou destino)
        sel = key - '1';

    if (key == '=')
    {
        fator += 5;
        processa();
    }
    if (key == '-')
    {
        fator -= 5;
        processa();
    }
    glutPostRedisplay();
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/Pixel.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
