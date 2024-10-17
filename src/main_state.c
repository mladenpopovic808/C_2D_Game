#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <rafgl.h>
#include <string.h>

int w,h;
static rafgl_raster_t raster;
static rafgl_texture_t tex;
rafgl_raster_t backgroundPictures[5];
int c=0;
int x,y;
int backgroundOption=0;
int tmp;
rafgl_pixel_rgb_t color1,colooriginalR,ballColor,targetColor,ballColooriginalR,topLeftColor,topRightColor,botLeftColor,botRightColor,A,B,example,result,blackColor;
rafgl_pixel_rgb_t sampled,linesColor;

///da bi jump bio smooth,a ne samo da mu naglo smanjim y,koristim jumpValue,koji sluzi da u svakoj iteraciji update-a,malo smanjim y
int jumpValue=0;
double horizontalSpeed=0;
double verticalSpeed=2;
float xn;
float ynn;
float cx,cy;
int cxTarget,cyTarget,cxTargetPrev,cyTargetPrev;
int verticalDirection;
int r;
double gravityCoef=0.1;
int avg;
float shadowCoef;
int sudar=0;
int originalR;
int changingCircleSize=0;


void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    w=512;
    h=512;
    r=w/11;
    originalR=w/11;
    rafgl_raster_init(&raster,w,h);
    rafgl_texture_init(&tex);
    cx=w/2;
    cy=h/2;///Koordinate naseg kruga
    cxTarget=w/3;///koordinate targeta
    cyTarget=h/3;


    for(int i=0;i<5;i++){
        char path[50];
        sprintf(path, "res\\img%d.jpg", i);
        rafgl_raster_load_from_image(&backgroundPictures[i], path);
    }

    color1.rgba=rafgl_RGB(0,255,0);
    colooriginalR.rgba=rafgl_RGB(0,80,200);
    ballColor.rgba=rafgl_RGB(255,0,0);
    targetColor.rgba=rafgl_RGB(220,20,60);
    ballColooriginalR.rgba=rafgl_RGB(255,255,0);
    topLeftColor.rgba=rafgl_RGB(0,0,230);
    topRightColor.rgba=rafgl_RGB(0,255,127);
    botLeftColor.rgba=rafgl_RGB(255,20,147);
    botRightColor.rgba=rafgl_RGB(255,165,0);
    blackColor.rgba=rafgl_RGB(0,0,0);
    linesColor.rgba=rafgl_RGB(255,255,0);

}
///Popunjavamo pozadinu
///imamo MOD-1 tipa pozadine:
#define MOD 7
void fillRaster(float xn,float yn,int x,int y){

    switch (backgroundOption){
///grey noise
case 0:
    tmp=rand()%256;
    pixel_at_m(raster,x,y).rgba=rafgl_RGB(tmp,tmp,tmp);
break;
case 1:
        ///negativ,delim sa 3 da bi bi se sporije menjala vrednost boja
        sampled=pixel_at_m(backgroundPictures[0],x,y);
        result.r=((int)cy/3)-sampled.r;
        result.g=((int)cy/3)-sampled.g;
        result.b=((int)cy/3)-sampled.b;
        pixel_at_m(raster,x,y)=result;

break;
///interpolacija
case 2:
    pixel_at_m(raster,x,y)=rafgl_lerppix(color1,colooriginalR,xn);
break;

case 3:
    ///mrak
    if(rafgl_distance2D(cx,cy,x,y)<2*r){
    pixel_at_m(raster,x,y)=pixel_at_m(backgroundPictures[2],x,y);

    }else{
    pixel_at_m(raster,x,y).rgba=blackColor.rgba;
    }
break;
///laguje,zakomentarisati po potrebi
case 4:
    A=rafgl_lerppix(topLeftColor,topRightColor,xn);
    B=rafgl_lerppix(botLeftColor,botRightColor,xn);
    pixel_at_m(raster,x,y)=rafgl_lerppix(A,B,yn);
    break;

///brigthness
case 5:
    sampled=pixel_at_m(backgroundPictures[3],x,y);
    result.r=rafgl_clampi(rafgl_saturatei(sampled.r-cy/2),0,255);
    result.g=rafgl_clampi(rafgl_saturatei(sampled.g-cy/2),0,255);
    result.b=rafgl_clampi(rafgl_saturatei(sampled.b-cy/2),0,255);
    pixel_at_m(raster,x,y)=result;

break;
case 6:
 pixel_at_m(raster,x,y)=pixel_at_m(backgroundPictures[4],x+(int)cx,y+(int)cy/2);

break;
}
}
int lineVal=0;
int lineValOffset=8;

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    ///postepeno pocevavanje kruga,nakon sudara
    if(changingCircleSize){
        r++;
        if(r==originalR){
            changingCircleSize=0;
        }
    }
    for(y=0;y<h;y++){
            ynn=1.0f*y/h;
        for(x=0;x<w;x++){
            ///xn i yn cuvam zbog interpolacije jedne od pozadina
            xn=1.0f*x/w;
            float distance=rafgl_distance2D(cx,cy,x,y);

                if(distance>r){
                   fillRaster(xn,ynn,x,y);
                }else{
                    ///nas krug
                    ///interpoliramo izmedju crvene i zelene

                    pixel_at_m(raster,x,y)=rafgl_lerppix(ballColor,ballColooriginalR,distance/r);

                }
                 if(rafgl_distance2D(cxTarget,cyTarget,x,y)<r*1.5f){
                    ///target
                    pixel_at_m(raster,x,y).rgba=targetColor.rgba;
                }
        }
    }
    ///povlaci linije iz centra ka coskovima
    ///ako se desio sudar
     if(sudar){

        pullBackCornerLines();
        spreadParticles();

    }else{
        ///crtaj linije ka centru kruga
        drawCornerLines();
    }

    ///Ako pogodimo target,menjaju mu se pozicije.
    ///Pogadjamo ga tako sto naletimo na njegovu boju
     if(pixel_at_m(raster,(int)cx,(int)cy).rgba==targetColor.rgba){

            backgroundOption=(backgroundOption+1)%MOD;///menjamo tip pozadine
            if(sudar==0){
            cxTargetPrev=cxTarget;
            cyTargetPrev=cyTarget;

            }
            sudar=1;
            changingCircleSize=1;
            cxTarget=rand()%(w/2)+r*2;///da bi target bio u vidljivom delu
            cyTarget=rand()%(w/2)+r*2;
            r=0;
        }


    ///dok postoji jump vrednost,smanjuj mu y,tj podizi krug na gore
    if(jumpValue>0){

        cy=cy*0.980f;
        jumpValue--;


    ///vertikalna brzina
    }else{
        cy+=verticalSpeed;

        if(verticalSpeed>0){

            verticalSpeed+=gravityCoef;

        }else if(verticalSpeed<0){///negativna je,znaci da se lopta uzdize na gore,i zelim da smanjim tu negativnu vrednost,da se sto pre vrati dole

                verticalSpeed+=4*gravityCoef;
        }
    }
    ///horizontalna brzina
    cx+=horizontalSpeed;

    if(horizontalSpeed>0){

        horizontalSpeed-=gravityCoef;

    }else if(horizontalSpeed<0){

        horizontalSpeed+=gravityCoef;
    }

    ///kada smo pritisnuli spejs,dodajemo mu jumpValue,koja sluzi za smanjivanje cy vrednosti pri svakoj update iteraciji.
    ///Ona se takodje smanjuje u svakoj update iteraciji,ako postoji(nije 0).To sluzi da skok izgleda smooth.
     if(game_data->keys_pressed[RAFGL_KEY_SPACE]){

      jumpValue+=25;
      verticalSpeed=-0.08f;


    }else if(game_data->keys_down[RAFGL_KEY_D])
    {
        cx+=3;
        horizontalSpeed+=2*gravityCoef;


   } else if(game_data->keys_down[RAFGL_KEY_A])
    {
        cx-=3;
        horizontalSpeed-=2*gravityCoef;


    }
    else if(game_data->keys_down[RAFGL_KEY_S])
    {
        cy+=5;
        verticalSpeed+=8*gravityCoef;
    }

    checkEdges();

}
///da li smo ispali iz okvira
void checkEdges(){

    ///levi zid
    if(cx-r-1 <= 0){

        cx=0+r+1;
        horizontalSpeed=-(horizontalSpeed);

    }
    ///analogno za "udaranje u pod"
    if(cy+r > h){

        cy=h-r;
        verticalSpeed=-(verticalSpeed);
    }
    ///plafon
    if(cy-r-1 <= 0){
        cy=0+r+2;
        verticalSpeed=-(verticalSpeed);
        jumpValue=0;
    }
    ///desni zid
    if(cx+r > w){

        cx = w-r;
        horizontalSpeed=-(horizontalSpeed);
       }
}

///crta linije iz coskova
void drawCornerLines(){

    int k=3;
    rafgl_raster_draw_line(&raster, cxTarget,cyTarget,0,0, linesColor.rgba);
    rafgl_raster_draw_line(&raster, cxTarget,cyTarget,w,0, linesColor.rgba);
    rafgl_raster_draw_line(&raster, cxTarget,cyTarget,0,w, linesColor.rgba);
    rafgl_raster_draw_line(&raster, cxTarget,cyTarget,w,w, linesColor.rgba);
}
void pullBackCornerLines(){
    ///gornji levi
    rafgl_raster_draw_line(&raster, cxTarget-lineVal,cyTarget-lineVal,0,0, linesColor.rgba);
    ///gornji desni
    rafgl_raster_draw_line(&raster, cxTarget+lineVal,cyTarget-lineVal,w,0, linesColor.rgba);
    ///donji levi
    rafgl_raster_draw_line(&raster, cxTarget-lineVal,cyTarget+lineVal,0,w, linesColor.rgba);
    ///donji desni
    rafgl_raster_draw_line(&raster, cxTarget+lineVal,cyTarget+lineVal,w,w, linesColor.rgba);
    lineVal+=lineValOffset; // +=8

    ///ako je svaka linija van ekrana,postavljamo sudar na 0,i tada ce se linije ucrtati u centar target kruga
    if(cxTarget-lineVal<0 && cxTarget+lineVal>w && cyTarget+lineVal>h && cyTarget-lineVal<0){
        sudar=0;
        lineVal=0;
    }

}
void spreadParticles(){

///ispaljujemo partikle,prema gore,dole,levo i desno

    ///gore
    rafgl_raster_draw_line(&raster, cxTarget,cyTarget-lineVal,cxTarget,cyTarget-1.1*lineVal, linesColor.rgba);
    ///dole
    rafgl_raster_draw_line(&raster, cxTarget,cyTarget+lineVal,cxTarget,cyTarget+1.1*lineVal, linesColor.rgba);
    ///levo
    rafgl_raster_draw_line(&raster, cxTarget-lineVal,cyTarget,cxTarget-1.1*lineVal,cyTarget, linesColor.rgba);
    ///desno
    rafgl_raster_draw_line(&raster, cxTarget+lineVal,cyTarget,cxTarget+1.1*lineVal,cyTarget, linesColor.rgba);
    lineVal+=lineValOffset; // +=8



}

void main_state_render(GLFWwindow *window, void *args)
{
    rafgl_texture_load_from_raster(&tex,&raster);
    rafgl_texture_show(&tex,0);

}
void main_state_cleanup(GLFWwindow *window, void *args)
{

}



