#include "msx_fusion.h"
#include "vdp_sprites.h"
#include "original_assets.h"
#include "audio.h"

typedef unsigned char u8;
typedef unsigned int u16;
/* Globals are also exposed to the emulator's regression scripts. */
u8 world[4096], screen_buffer[768], sat[128], scanlines[192];
u16 px,py,ex[2],ey[2],score;
u8 floor_index,camx,camy,keys,ammo,hits,map_mode,tick,direction,enemy_hp[2];
static u8 mini_rows[1024],mini_pattern[1024],mini_color[1024];
static u8 cached_frame[3],sat_count,old_fire,hud_dirty;
static int bx,by;
static u8 bullet_dir,change_count;
static u16 changed_index[128];
static u8 changed_floor[128],changed_tile[128];
static const u8 shot_pattern[32]={0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,192,0,0,0,0,0,0,0};

static void frame(void){__asm halt __endasm;}
static u8 attribute(u8 t){return t<152?tile_attr[t]:1;}
static u16 position(u8 x,u8 y){return ((u16)y<<6)+x;}
static u8 at(u8 x,u8 y){return world[position(x,y)];}
static void put(u8 x,u8 y,u8 t){Vpoke(0x1800+((u16)y<<5)+x,t);}
static void text(u8 x,u8 y,char *s){while(*s)put(x++,y,(*s++)+128);}
static void number(u8 x,u8 y,u8 n){u8 tens=0;while(n>=10){n-=10;tens++;}put(x,y,176+tens);put(x+1,y,176+n);}
static void score_display(void){static const u16 powers[5]={10000,1000,100,10,1};u16 value=score;u8 i,d;put(25,11,176);for(i=0;i<5;i++){d=0;while(value>=powers[i]){value-=powers[i];d++;}put(26+i,11,176+d);}}
static void hide_sprites(void){Vpoke(0x1b00,208);}
static void load_art(void){u8 bank;for(bank=0;bank<3;bank++){
 CopyRamToVram((void*)tile_pattern,(u16)bank<<11,2048);
 CopyRamToVram((void*)tile_color,0x2000+((u16)bank<<11),2048);
}}
static void hud(void){
 text(25,2,"KEYS");number(29,3,keys);
 text(25,5,"AMMO");number(29,6,ammo);
 text(25,8,"HITS");number(29,9,hits);
 score_display();text(25,13,"FLOOR");number(28,14,floor_index);
 text(25,17,"M MAP");text(25,19,"E LIFT");text(25,21,"1-4");
 text(0,23,"                        ");
 if(map_mode)text(1,23,"MAP - M TO RETURN");
 else {if(floor_index==0)text(0,23,"BASEMENT");if(floor_index==1)text(0,23,"GROUND FLOOR");if(floor_index==2)text(0,23,"FIRST FLOOR");if(floor_index==3)text(0,23,"SECOND FLOOR");text(16,23,"EXPLORE");}
 hud_dirty=0;
}
static void draw_room(void){u8 x,y;u16 a=0;
 hide_sprites();
 for(y=0;y<24;y++)for(x=0;x<32;x++){
  if(x>=24)screen_buffer[a]=152;
  else if(y<23)screen_buffer[a]=at(camx+x,camy+y);
  else screen_buffer[a]=152;
  a++;
 }
 CopyRamToVram(screen_buffer,0x1800,768);hud();
}
static void camera(u8 force){int x=(int)camx,y=(int)camy;
 if(force){x=(px>>3)-11;y=(py>>3)-10;}
 else{
  if(px<((u16)camx<<3)+48)x=(px>>3)-6;
  if(px>((u16)camx<<3)+128)x=(px>>3)-16;
  if(py<((u16)camy<<3)+40)y=(py>>3)-5;
  if(py>((u16)camy<<3)+128)y=(py>>3)-16;
 }
 if(x<0)x=0;if(x>40)x=40;if(y<0)y=0;if(y>41)y=41;
 if(force||camx!=(u8)x||camy!=(u8)y){camx=x;camy=y;draw_room();}
}
static void change(u8 x,u8 y,u8 t){u16 a=position(x,y);u8 i;
 world[a]=t;
 {u16 ma=(((u16)(y>>2)<<4)+(x>>2))*4+(y&3);u8 mask=0xc0>>((x&3)<<1),kind=attribute(t);if(kind>=1&&kind<=4)mini_rows[ma]|=mask;else mini_rows[ma]&=~mask;}
 for(i=0;i<change_count;i++)if(changed_index[i]==a&&changed_floor[i]==floor_index){changed_tile[i]=t;break;}
 if(i==change_count&&change_count<128){changed_index[i]=a;changed_floor[i]=floor_index;changed_tile[i]=t;change_count++;}
 if(!map_mode&&x>=camx&&x<camx+24&&y>=camy&&y<camy+23)put(x-camx,y-camy,t);
}
static void clear_group(u8 x,u8 y,u8 kind){int xx,yy;
 for(yy=(int)y-1;yy<=(int)y+1;yy++)for(xx=(int)x-1;xx<=(int)x+1;xx++)
 if(xx>=0&&xx<64&&yy>=0&&yy<64&&attribute(at(xx,yy))==kind)change(xx,yy,6);
}
static u8 blocked(int x,int y,u8 open){u8 tx,ty,a;
 if(x<0||x>=512||y<0||y>=512)return 1;
 tx=x>>3;ty=y>>3;a=attribute(at(tx,ty));
 if(a==2&&open&&keys){clear_group(tx,ty,2);keys--;AudioEffect(FX_DOOR);hud_dirty=1;return 0;}
 return a>=1&&a<=4;
}
static u8 fits(int x,int y,u8 open){
 return !blocked(x+4,y+11,open)&&!blocked(x+11,y+11,open)&&!blocked(x+4,y+21,open)&&!blocked(x+11,y+21,open);
}
static void spawn_guards(void){u8 n,x,y,found;int wx,wy;
 for(n=0;n<2;n++){found=0;enemy_hp[n]=0;
 for(y=3+n*7;y<20&&!found;y++)for(x=3;x<21&&!found;x++){
 wx=((int)camx+x)<<3;wy=((int)camy+y)<<3;
 if(wx>(int)px-48&&wx<(int)px+48&&wy>(int)py-40&&wy<(int)py+40)continue;
 if(fits(wx,wy,0)){ex[n]=wx;ey[n]=wy;enemy_hp[n]=2;found=1;}
 }}
}
static void load_floor(u8 f,u8 initial){const u8 *p;u16 out=0,ma;u8 code,n,value,i,mx,my,kind;
 hide_sprites();floor_index=f;p=floor_data[f];
 while(out<4096){code=*p++;if(code&128){n=code-127;value=*p++;while(n--)world[out++]=value;}else{n=code+1;while(n--)world[out++]=*p++;}}
 for(out=0;out<1024;out++)mini_rows[out]=0;
 for(my=0;my<64;my++)for(mx=0;mx<64;mx++){
 kind=attribute(at(mx,my));if(kind>=1&&kind<=4){ma=(((u16)(my>>2)<<4)+(mx>>2))*4+(my&3);mini_rows[ma]|=0xc0>>((mx&3)<<1);}
 }
 for(i=0;i<change_count;i++)if(changed_floor[i]==f)change(changed_index[i]&63,changed_index[i]>>6,changed_tile[i]);
 if(initial){px=272;py=472;}else{px=72;py=464;}
 bx=-1;camx=0;camy=0;for(i=0;i<3;i++)cached_frame[i]=255;
 camera(1);spawn_guards();
}
static void pickup(void){u8 x=(px+8)>>3,y=(py+16)>>3,a=attribute(at(x,y));
 if(a==5){if(keys<99)keys++;score+=50;clear_group(x,y,a);AudioEffect(FX_PICKUP);hud_dirty=1;}
 if(a==6){score+=250;clear_group(x,y,a);AudioEffect(FX_PICKUP);hud_dirty=1;}
 if(a==7){ammo=99;score+=25;clear_group(x,y,a);AudioEffect(FX_PICKUP);hud_dirty=1;}
 if(a==8){hits=0;score+=25;clear_group(x,y,a);AudioEffect(FX_PICKUP);hud_dirty=1;}
}
static void sprite(u8 x,int y,u8 pattern,u8 color){u8 i,start,end;
 if(y<0||y>=184||sat_count>=31)return;
 start=y;end=y+16;if(end>192)end=192;
 for(i=start;i<end;i++)if(scanlines[i]>=4)return;
 for(i=start;i<end;i++)scanlines[i]++;
 i=sat_count<<2;sat[i]=(u8)(y-1);sat[i+1]=x;sat[i+2]=pattern;sat[i+3]=color;sat_count++;
}
static void actor_layer(u8 actor,u16 x,u16 y,u8 layer,u8 color){int sx=(int)x-((u16)camx<<3),sy=(int)y-((u16)camy<<3);u8 base;
 if(sx<0||sx>176||sy<0||sy>160)return;
 base=actor*32+layer*8;sprite(sx,sy,base,color);sprite(sx,sy+16,base+4,color);
}
static void upload_actor(u8 actor,u8 f){if(cached_frame[actor]==f)return;
 CopyRamToVram((void*)actor_frames[f],0x3800+((u16)actor<<8),256);cached_frame[actor]=f;
}
static void draw_actors(void){u8 i,n,ed,ef;int dx,dy,sx,sy;
 sat_count=0;for(i=0;i<192;i++)scanlines[i]=0;
 upload_actor(0,direction+((tick>>3)&1));
 for(n=0;n<2;n++)if(enemy_hp[n]){
 dx=(int)px-ex[n];dy=(int)py-ey[n];if(dx<0)dx=-dx;if(dy<0)dy=-dy;
 if(dx>dy)ed=px<ex[n]?4:6;else ed=py<ey[n]?2:0;
 ef=8+ed+((tick>>3)&1);upload_actor(n+1,ef);
 }
 actor_layer(0,px,py,0,12);
 for(n=0;n<2;n++)if(enemy_hp[n])actor_layer(n+1,ex[n],ey[n],0,14);
 if(bx>=0){sx=bx-((u16)camx<<3)-7;sy=by-((u16)camy<<3)-7;if(sx>=0&&sx<176)sprite(sx,sy,112,11);}
 actor_layer(0,px,py,3,1);
 for(n=0;n<2;n++)if(enemy_hp[n])actor_layer(n+1,ex[n],ey[n],3,1);
 actor_layer(0,px,py,1,10);
 for(n=0;n<2;n++)if(enemy_hp[n])actor_layer(n+1,ex[n],ey[n],1,10);
 actor_layer(0,px,py,2,3);
 for(n=0;n<2;n++)if(enemy_hp[n])actor_layer(n+1,ex[n],ey[n],2,15);
 sat[sat_count<<2]=208;CopyRamToVram(sat,0x1b00,(sat_count<<2)+1);
}
static void overview(void){u8 x,y,my,yy,bit,id,bank,marker;u16 a,b,i;
 hide_sprites();
 for(i=0;i<768;i++)screen_buffer[i]=(i&31)>=24?152:153;
 for(bank=0;bank<3;bank++){
  for(i=0;i<1024;i++){mini_pattern[i]=0;mini_color[i]=0x71;}
  for(y=0;y<8;y++){
   my=bank*8+y;if(my<3||my>=19)continue;my-=3;
   for(x=0;x<16;x++){
    id=(y<<4)+x;a=(u16)id<<3;b=(((u16)my<<4)+x)<<2;
    marker=x==(px>>5)&&my==(py>>5);
    for(yy=0;yy<4;yy++){
     bit=marker?255:mini_rows[b+yy];mini_pattern[a]=bit;mini_pattern[a+1]=bit;
     mini_color[a]=marker?0x81:0x71;mini_color[a+1]=marker?0x81:0x71;a+=2;
    }
    screen_buffer[((u16)(my+3)<<5)+x+4]=id;
   }
  }
  a=(u16)bank<<11;CopyRamToVram(mini_pattern,a,1024);CopyRamToVram(mini_color,0x2000+a,1024);
 }
 CopyRamToVram(screen_buffer,0x1800,768);text(3,0,"CASTLE PLAN");hud();
}
static void bullet_step(void){u8 t,a,n;int dx,dy;
 if(bx<0)return;
 if(bullet_dir==2)by-=4;else if(bullet_dir==0)by+=4;else if(bullet_dir==4)bx-=4;else bx+=4;
 if(bx<0||by<0||bx>=512||by>=512){bx=-1;return;}
 t=at(bx>>3,by>>3);a=attribute(t);
 if(a>=1&&a<=4){if(a==3){clear_group(bx>>3,by>>3,3);score+=10;hud_dirty=1;}bx=-1;return;}
 if(a==9){hits=49;hud_dirty=1;bx=-1;return;}
 for(n=0;n<2;n++)if(enemy_hp[n]){dx=bx-ex[n];dy=by-ey[n];if(dx>=0&&dx<16&&dy>=0&&dy<24){enemy_hp[n]--;if(!enemy_hp[n]){score+=100;AudioEffect(FX_KILL);hud_dirty=1;}bx=-1;return;}}
}
void main(void){u8 joy,key,fire,n;int nx,ny,dx,dy;
 SetColors(15,1,1);Screen(2);SpriteReset();Sprite16();SpriteSmall();load_art();
 CopyRamToVram((void*)shot_pattern,0x3b80,32);
 AudioInit();
restart: keys=0;ammo=99;hits=0;score=0;change_count=0;tick=0;old_fire=0;map_mode=0;direction=2;load_floor(1,1);
 for(;;){frame();tick++;key=Inkey();
  if(key=='v'||key=='V'){AudioToggle();continue;}
  if(key==27){load_art();goto restart;}
  if(key=='m'||key=='M'){
   map_mode=!map_mode;if(map_mode)overview();else{load_art();draw_room();}continue;
  }
  if(key>='1'&&key<='4'){if(map_mode)load_art();load_floor(key-'1',0);if(map_mode)overview();continue;}
  if(map_mode)continue;
  if((key=='e'||key=='E')&&(px>>3)>=6&&(px>>3)<=12&&(py>>3)>=56){load_floor((floor_index+1)&3,0);continue;}
  joy=JoystickRead(0);if(!joy)joy=JoystickRead(1);
  nx=px;ny=py;
  if(joy){if(joy==1||joy==2||joy==8){ny-=2;direction=2;}else if(joy>=4&&joy<=6){ny+=2;direction=0;}
   if(joy>=2&&joy<=4){nx+=2;direction=6;}else if(joy>=6&&joy<=8){nx-=2;direction=4;}
   if(fits(nx,py,1))px=nx;if(fits(px,ny,1))py=ny;
  }
  pickup();camera(0);
  fire=TriggerRead(0)|TriggerRead(1);if(fire&&!old_fire&&bx<0&&ammo){bx=px+8;by=py+12;bullet_dir=direction;ammo--;AudioEffect(FX_SHOT);hud_dirty=1;}old_fire=fire;
  bullet_step();
  if(!(tick&3))for(n=0;n<2;n++)if(enemy_hp[n]){
   dx=(int)px-ex[n];dy=(int)py-ey[n];nx=ex[n];ny=ey[n];
   if(dx<0)nx--;if(dx>0)nx++;if(dy<0)ny--;if(dy>0)ny++;
   if(fits(nx,ey[n],0))ex[n]=nx;if(fits(ex[n],ny,0))ey[n]=ny;
   if(dx>-14&&dx<14&&dy>-20&&dy<20&&!(tick&31)){hits++;AudioEffect(FX_HIT);hud_dirty=1;}
  }
  if(hits>=50)goto restart;
  if(hud_dirty)hud();draw_actors();
 }
}
