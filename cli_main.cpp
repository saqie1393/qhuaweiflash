#include <QtWidgets>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include "ptable.h"
#include "signver.h"

ptable_list *ptable = nullptr;
QString fwfilename;
int modified = 0;
void set_modified(){ modified=1; }

static void usage() {
  puts("qhuaweiflash-cli - firmware image editor (NO flashing)");
  puts("Usage:");
  puts("  qhuaweiflash-cli info <firmware>");
  puts("  qhuaweiflash-cli list <firmware>");
  puts("  qhuaweiflash-cli extract <firmware> <PART-ID|index> <output>");
  puts("  qhuaweiflash-cli replace <firmware> <PART-ID|index> <input> <output>");
  puts("  qhuaweiflash-cli delete <firmware> <PART-ID|index> <output>");
  puts("  qhuaweiflash-cli move <firmware> <PART-ID|index> up|down <output>");
  puts("PART-ID can be hex (e.g. 0x30000); index is zero-based.");
}
static bool readfile(const char *p, std::vector<uint8_t>& b) {
 std::ifstream f(p,std::ios::binary); if(!f)return false;
 b.assign(std::istreambuf_iterator<char>(f),{}); return true;
}
static bool writefile(const char *p,const uint8_t *d,size_t n) {
 std::ofstream f(p,std::ios::binary); if(!f)return false;
 f.write((const char*)d,n); return !!f;
}
static int partarg(const char *s) {
 char *e=nullptr; unsigned long v=strtoul(s,&e,0);
 if(e && *e==0) {
   for(int i=0;i<ptable->index();i++) if(ptable->code(i)==v)return i;
   if(v<(unsigned long)ptable->index()) return (int)v;
 }
 return -1;
}
static bool loadfw(const char *path) {
 FILE *f=fopen(path,"rb"); if(!f){perror(path);return false;}
 ptable->clear(); dload_id=-1; ptable->findparts(f); fclose(f); fwfilename=path;
 return ptable->index()>0;
}

static bool savefw(const char *path) {
 FILE *o=fopen(path,"wb");
 if(!o){perror(path);return false;}

 // CLI output is unsigned firmware
 uint8_t hdr[92]={0};
 hdr[0]=dload_id&7;

 bool ok=fwrite(hdr,1,sizeof(hdr),o)==sizeof(hdr);

 for(int i=0;ok && i<ptable->index();i++)
   ptable->save_part(i,o,false);

 if(fclose(o)!=0)ok=false;
 return ok;
}

int main(int argc,char **argv) {
 qputenv("QT_QPA_PLATFORM","offscreen");
 QApplication app(argc,argv);
 if(argc<3){usage();return 2;}
 const char *cmd=argv[1],*fw=argv[2];
 ptable=new ptable_list();
 if(!loadfw(fw)){fprintf(stderr,"Could not parse firmware (or no partitions found).\\n");return 1;}
 if(!strcmp(cmd,"info")||!strcmp(cmd,"list")){
   printf("Firmware: %s\\nType: %s (0x%x)\\nPartitions: %d\\n",fw,fw_description(dload_id),dload_id,ptable->index());
   for(int i=0;i<ptable->index();i++)
    printf("[%03d] id=0x%08x name=%s size=%u type=%d\\n",i,ptable->code(i),ptable->name(i),ptable->psize(i),ptable->ptype(i));
   return 0;
 }
 if(!strcmp(cmd,"extract")){
   if(argc!=5){usage();return 2;} int n=partarg(argv[3]); if(n<0){puts("Invalid partition");return 2;}
   if(!writefile(argv[4],ptable->iptr(n),ptable->psize(n))){perror(argv[4]);return 1;} return 0;
 }
 if(!strcmp(cmd,"replace")){
   if(argc!=6){usage();return 2;} int n=partarg(argv[3]); std::vector<uint8_t>b;
   if(n<0||!readfile(argv[4],b)){puts("Invalid partition/input");return 2;}
   ptable->replace(n,b.data(),(uint32_t)b.size()); if(!savefw(argv[5]))return 1; return 0;
 }
 if(!strcmp(cmd,"delete")){
   if(argc!=5){usage();return 2;} int n=partarg(argv[3]);if(n<0)return 2;
   ptable->delpart(n); if(!savefw(argv[4]))return 1;return 0;
 }
 if(!strcmp(cmd,"move")){
   if(argc!=6){usage();return 2;} int n=partarg(argv[3]);if(n<0)return 2;
   if(!strcmp(argv[4],"up"))ptable->moveup(n);else if(!strcmp(argv[4],"down"))ptable->movedown(n);else {usage();return 2;}
   if(!savefw(argv[5]))return 1;return 0;
 }
 usage(); return 2;
}
