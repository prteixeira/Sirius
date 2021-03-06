/* Khole OS v0.3
 * Copyright (C) 2017,2018 (Nelson Sapalo da Silva Cole)
 */
#include <io.h>
#include <string.h>
#include <fs/fs.h>

#define FAT12 12
#define FAT16 16
#define FAT32 32
#define ExFAT 0

#define FAT_LFN 0x80
#define FAT_SFN 0x00


#define FAT_ATTR_READ_ONLY	0x01
#define FAT_ATTR_HIDDEN		0x02
#define FAT_ATTR_SYSTEM		0x04
#define FAT_ATTR_VOLUME_ID	0x08
#define FAT_ATTR_DIRECTORY	0x10
#define FAT_ATTR_ARCHIVE		0x20
#define FAT_ATTR_LONG_NAME	(FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN\
 				| FAT_ATTR_SYSTEM| FAT_ATTR_VOLUME_ID)

#define FAT_DIR_ENTRY_SIZE	32

struct bpb_s 
{

	char BS_jmpBoot[3];
	char BS_OEMName[8];
	uint16_t BPB_BytsPerSec;
	uint8_t BPB_SecPerClus;
	uint16_t BPB_RsvdSecCnt;
	uint8_t BPB_NumFATs;
	uint16_t BPB_RootEntCnt;
	uint16_t BPB_TotSec16;
	uint8_t BPB_Media;
	uint16_t BPB_FATSz16;
	uint16_t BPB_SecPertrk;
	uint16_t BPB_NumHeads;
	uint32_t BPB_HiddSec;
	uint32_t BPB_TotSec32;
	
union {
 	struct 
    {// Para FAT12/FAT16

	uint8_t BS_DrvNum;
    	uint8_t BS_Reserved1;
    	uint8_t BS_BootSig;
	uint32_t BS_VolID;
 	char BS_VolLab[11];
	char BS_FilSysType[8];
		
	}__attribute__ ((packed)) fat12_or_fat16;

            struct 
            { // Para FAT32
	
	            uint32_t BPB_FATSz32;
	            uint16_t BPB_ExtFlags;
	            uint16_t BPB_FSVer;
	            uint32_t BPB_RootClus;
	            uint16_t BPB_FSInfo;
	            uint16_t BPB_BkBootSec;
	            char BPB_Reserved[12];
	            uint8_t BS_DrvNum;
	            uint8_t BS_Reserved1;
	            uint8_t BS_BootSig;
	            uint32_t BS_VolID;
	            char BS_VolLab[11];
	            char BS_FilSysType[8];

	        }__attribute__ ((packed)) fat32;
	}__attribute__ ((packed)) specific;
}__attribute__ ((packed));

struct directory_s
{
	
	char DIR_Name[8];
	char DIR_Name_Ext[3];
	uint8_t DIR_Attr;
	uint8_t DIR_NTRes;
	uint8_t DIR_CrtTimeTenth;
	uint16_t DIR_CrtTime;
	uint16_t DIR_CrtDate;
	uint16_t DIR_LstAccDate;
	uint16_t DIR_FstClusHI;
	uint16_t DIR_WrtTime;
	uint16_t DIR_WrtDate;
	uint16_t DIR_FstClusLO;
	uint32_t DIR_FileSize;
	
}__attribute__ ((packed));



struct data_s
{
    	uint32_t count_of_clusters;
    	uint32_t data_sectors;
    	uint32_t first_data_sector;
    	uint32_t first_fat_sector;
    	uint32_t fat_offset;
    	uint32_t first_root_directory_sector_num;
    	uint32_t first_sector_of_cluster;
    	uint32_t fat_size;
    	uint32_t fat_type;
    	uint32_t root_cluster;
    	uint32_t root_directory_sectors;
    	uint32_t volume_total_sectors;

    	uint32_t fat_total_sectors;
    	uint32_t fat_ent_offset;
  


}__attribute__ ((packed));






struct directory_s st_directory;
struct bpb_s bpb;
struct data_s *data;


static uint32_t fat_path_count(char *path)
{
    	uint32_t    val =0;
    	char     a='/';
    	int    i;
    	for(i=0;i < 0xFFFF;i++)
	{
    		if(a == path[i])val++;
    		if(path[i] =='\0')break;
    	}

    	return val;
}

static uint32_t fat_path_offset(char *path)
{
    	uint32_t    val=0;
    	while(path[++val]);
 
    	return val;

}

static char *fat_path_copy(char *s1,char *s2)
{
    	char *s1_p = (char*)s1;
    	int i =0;
    	while (s2[i] != '/')
	{
        	s1[i] = s2[i];
        	i++;
    	}
    	s1[i] ='\0';
    	return s1_p;
            

}

static uint32_t read_fat(char *fat_buf,uint32_t N,uint8_t dev)
{

    	uint32_t ret_val = 0;
    	int a,b,c;
    	uint64_t lba_start;

    	switch(data->fat_type){
    	case 12:
        a = N /2730;
        c = N %2730;
    
        lba_start = bpb.BPB_RsvdSecCnt + (8*a);
        read_sector(fat_buf,lba_start,8,dev);       
        if((c%2)==0){

                ret_val = fat_buf [c+(c/2)];
                ret_val = ret_val | (fat_buf [c + (c/2) + 1 ] << 8);
                ret_val = ret_val &0xfff;
                }                   
           else {

        
                ret_val = fat_buf [c+(c/2)];
                ret_val = ret_val | (fat_buf [c + (c/2) + 1 ] << 8);
                ret_val = ret_val >> 4 &0xfff;
                }
        break;
    	case 16:
        a = N /2048;
        c = N %2048;
        lba_start = bpb.BPB_RsvdSecCnt + (8*a);
        read_sector(fat_buf,lba_start,8,dev);               
        ret_val = (*(uint16_t*)(fat_buf + (c * 2))); 
        break;
    	case 32:
        a = N /1024;
        c = N %1024;
        lba_start = bpb.BPB_RsvdSecCnt + (8*a);
        read_sector(fat_buf,lba_start,8,dev);
        ret_val = (*(uint32_t*)(fat_buf + (c * 4)));        
        break;
    	}

    	return ret_val;
}


static uint32_t fat_read_dir(char *in, char *out,char *filename)
{
    	uint32_t offs_out =256;
    	uint32_t offs_in = 0;
    	char  buffer_tmp [128];
    	int i,t = 1,c,a,b;

    	//Assinatura
    	strcpy(out,"DIR \0");

    	while(TRUE)
{   
    
    if(in[1 + offs_in]==0)return 0; // FIM
     
    if(in[1+ offs_in]==0xE5) goto goto_2; //Entrada não será utilizada
    if(in[0+ offs_in]=='.')  goto goto_2; //Entrada não '.' e '..'

    if(in[11 + offs_in]==FAT_ATTR_LONG_NAME)
    {//Se igual a 0xF, entrada de LFN

    if (t){b = in[offs_in] &0xf;t =0;}
    a = in[offs_in];

    strncpy( buffer_tmp + (32 *((a&0xf) - 1)) ,in + offs_in,32);
    goto goto_2;

    }
    else
    { // Entrada padrão 8.3

    // extrair os dados para mais tarde poder utilizar.
    strncpy( (char*)&st_directory,in + offs_in,32);
    }    
         
   
    // Existe um nome de arquivo longo no buffer temporário?
    if(buffer_tmp[11] == 0xF)
{
    //compara nome de arquivo LFN
    c = 0;
    for(t=0;t < b;++t)
{
    int index = 1;
    for(i=1;i < 11; i++)
    {
        if((buffer_tmp[i + (t*32) +1] == 0) && ((*(uint16_t*)(buffer_tmp + i + (t*32) -1)) != 0xFFFF))
        {
            filename[c] = buffer_tmp[index + (t*32)];
            c++;
        }
        index++;
    }
    index = 14;
    for(i=14;i < (14 +11); i++)
    {
        if((buffer_tmp[i + (t*32) + 1 ] == 0) && ((*(uint16_t*)(buffer_tmp +i + (t*32) -1)) != 0xFFFF))
        {
            filename[c] = buffer_tmp[index + (t*32)];
            c++;
        }
        index++;
    }
    index =28;
    for(i=28;i < 32; i++)
    {
        if((buffer_tmp[i + (t*32) +1] == 0) && ((*(uint16_t*)(buffer_tmp +i + (t*32) -1)) != 0xFFFF))
        {
    
            filename[c] = buffer_tmp[index + (t*32)]; 
            c++;
        }
        index++;
     }
}
    filename[c] ='\0';
}
    else
{
    // Nome de arquivo padrao 8.3
    strncpy(filename,(char*)&st_directory,11);
}
    
    (*(uint32_t*)(out + 4)) += 1;     
    strcpy(out + offs_out,filename);
    strncpy(out + offs_out + 128,(char*)&st_directory,32);
    offs_out +=256;

goto_2:
    offs_in +=32;
}

}


// FUNCOES A FAT

int fat_filesystem(char *path,uint8_t dev,void *out,uint8_t flg)
{
    	uint8_t     dir =0;
    	int    	    RET_VAL = -1;
    	uint32_t    offset = 0;
    	uint32_t    N;
    	uint32_t    EOF;
    	uint32_t    EOF8;
    	uint32_t    eof =1;
    	uint32_t    sector_count;
    	uint64_t    lba_start = 0;
    	uint32_t    table_value;
    	int	    i,t = 1,c,a,b;
    	char        buffer_tmp [256];
    	char        *filename =(char*)malloc(256); 
    	char        *search;
    	uint32_t    offset_dir  = 0;
    	int         count_path  =0;
    	int         offset_path =0;

    	uint32_t    out_offset  = 0;
    	uint8_t     w = 1;
    	uint32_t    out_data;
    
    	char       *path0 =(char*)malloc(0x1000);
    	char       *directory =(char*)malloc(4096*8); //TODO Aqui vamos alocar 4MB
    	char       *fat_buf =(char*)malloc(0x1000);
    	char        buf[512];


    	data = (struct data_s*)malloc(sizeof(struct data_s));


    	if(*path == '\0')dir =1;

    	if((read_sector(buf,0,1,dev))!=0){
    
        printf("Erro na FAT\n");
        RET_VAL = -1;
        goto end;
    	}

    
    	strncpy((char*)&bpb,buf,90);
     
    /*TODO Determina o tipo de FAT montado no volume, o tipo de fat é determinado pelo número total de clusters em partiçao
     O BPB_RootEntCnt para fat32 e sempre zero */

    	data->root_directory_sectors =((bpb.BPB_RootEntCnt * 32)+(bpb.BPB_BytsPerSec - 1))/bpb.BPB_BytsPerSec; 
    	data->fat_size = (bpb.BPB_FATSz16 == 0)? bpb.specific.fat32.BPB_FATSz32 : bpb.BPB_FATSz16;
    	data->volume_total_sectors = (bpb.BPB_TotSec16 == 0)? bpb.BPB_TotSec32 : bpb.BPB_TotSec16;
    	data->data_sectors = data->volume_total_sectors - (bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * data->fat_size)\
    	+  data->root_directory_sectors); 
    	data->count_of_clusters = data->data_sectors / bpb.BPB_SecPerClus;


    	if (data->count_of_clusters < 4085 && bpb.BPB_FATSz16 != 0)
    	{
        	data->fat_type = FAT12;

    	}
    	else if (data->count_of_clusters < 65525 && bpb.BPB_FATSz16 != 0)
    	{
        	data->fat_type = FAT16;

    	}
    	else if (data->count_of_clusters < 268435445)
    	{
        	data->fat_type = FAT32;

    	}
    	else 
    	{
        	data->fat_type = ExFAT;
        	printf("Sem suporte, o volume deve ser ExFAT");
        	RET_VAL = -1;
        	goto end;
    	}

    	/*determine FAT offset, tamanho da FAT em bytes */
    	switch (data->fat_type){
    	case 12:
            	data->fat_offset = data->fat_size + (data->fat_size/2); // div 1.5
            	break;
   	case 16:
            	data->fat_offset = data->fat_size * 2;
            	break;
    	case 32:
            	data->fat_offset = data->fat_size * 4;
            	break;
    	default:
            // O volume eve ser ExFAT
            break;
    	}
    
    	//Feito
    	//TODO Nelson, aqui se pode calcular o tamanho da FAT, yep, mas nao estamos interessados.

    	/* Tamanho da FAT em sectors  */
    	data->fat_total_sectors = bpb.BPB_RsvdSecCnt + (data->fat_offset / bpb.BPB_BytsPerSec);
    	data->fat_ent_offset = data->fat_offset % bpb.BPB_BytsPerSec;    
    	/*   FIXME Read FAT*/
    	data->first_data_sector = bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * data->fat_size) + data->root_directory_sectors;
    	data->root_cluster = bpb.specific.fat32.BPB_RootClus;

    	// Lendo a File Allocation Table 12, 16 ou 32
    	sector_count = data->fat_total_sectors;

    	// Lendo o Root Directory
    	if (data->fat_type == FAT12 || data->fat_type == FAT16)
    	{        
        	data->first_root_directory_sector_num = data->first_data_sector - data->root_directory_sectors;
        	sector_count = data->root_directory_sectors;
        	lba_start = data->first_root_directory_sector_num;
        	read_sector(directory,lba_start,sector_count,dev);
                  
    	}
    	else if (data->fat_type == FAT32)
    	{
        	EOF     = 0x0FFFFFFF;
        	EOF8    = 0x0FFFFFF8;
        	// Lendo o first Directory ou Root Directory

        	N = data->root_cluster;
        	sector_count = bpb.BPB_SecPerClus;
        	eof = 1;

    	do{
        	data->first_sector_of_cluster = ((  N -2)*bpb.BPB_SecPerClus) + data->first_data_sector;
        	lba_start = data->first_sector_of_cluster;
        	read_sector(directory + offset,lba_start,sector_count,dev);

        	table_value = read_fat(fat_buf,N,dev); 
        
        	if ((table_value !=EOF) && (table_value != EOF8))
        	{ 
        	N = table_value;
        	offset =  offset + (bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus);
        	}
        	else offset = 0; 


        	if((table_value ==EOF) || (table_value == EOF8))
        	eof =0;
            
        	}while(eof);

    	}


    	//TODO aqui saberes se o acesso e de apenas a leitura do directorio raiz


    
    	//FUNCAO FAT READ/WRITE DIRECTORY/FILE
    	// Se flg == 0 Read
    	// Se flg == 1 Write
    	if(flg == 0){

    	count_path = fat_path_count(path);

goto_0:    
    	fat_path_copy(path0,path + offset_path);

    	if(*path0 == 0) dir =1;

    	if(dir){

            	fat_read_dir(directory,out,filename);
            	RET_VAL = 0; goto end;
            
     	}



    	// FIXME Aqui vamos ler os arquivos a partir dos dados do directório raíz
    	search = directory; 
    	offset_dir = 0;
goto_1:
    	if(search[1 + offset_dir]==0){RET_VAL = -1; goto end;} // FIXME errro
     
    	if(search[1+ offset_dir]==0xE5) goto goto_2; //Entrada não será utilizada

    	if(search[11 + offset_dir]==FAT_ATTR_LONG_NAME) {//Se igual a 0xF, entrada de LFN

    	if (t){b = search[offset_dir] &0xf;t =0;}
    	a = search[offset_dir];

    	strncpy(buffer_tmp + (32 *((a&0xf) - 1)) ,search + offset_dir,32);
    	goto goto_2;

    	}
    	else
    	{ // Entrada padrão 8.3

    	// extrair os dados para mais tarde poder utilizar.
    	strncpy( (char*)&st_directory,search + offset_dir,32);
    	}    
         
   
    	// Existe um nome de arquivo longo no buffer temporário?
    	if(buffer_tmp[11] == 0xF)
    	{
    	//compara nome de arquivo LFN
    	c = 0;
    	for(t=0;t < b;++t)
    	{
    	int index = 1;
    	for(i=1;i < 11; i++)
    	{
        if((buffer_tmp[i + (t*32) +1] == 0) && ((*(uint16_t*)(buffer_tmp + i + (t*32) -1)) != 0xFFFF))
        {
            filename[c] = buffer_tmp[index + (t*32)];
            c++;
        }
        index++;
    	}
    	index = 14;
    	for(i=14;i < (14 +11); i++)
    	{
        if((buffer_tmp[i + (t*32) + 1 ] == 0) && ((*(uint16_t*)(buffer_tmp +i + (t*32) -1)) != 0xFFFF))
        {
            filename[c] = buffer_tmp[index + (t*32)];
            c++;
        }
        index++;
    	}
    	index =28;
    	for(i=28;i < 32; i++)
    	{
        if((buffer_tmp[i + (t*32) +1] == 0) && ((*(uint16_t*)(buffer_tmp +i + (t*32) -1)) != 0xFFFF))
        {
    
            filename[c] = buffer_tmp[index + (t*32)]; 
            c++;
        }
        index++;
     	}
    	}
        filename[c] ='\0';
    	}
    	else
    	{
    	// Nome de arquivo padrao 8.3
    	strncpy(filename,(char*)&st_directory,11);
    	}

    	//FIXME aqui vamos ler o sub directorio ou o arquivo seguindo cadei de cluster
    	//TODO
    
    	if((strcmp(filename,path0)) == 0){
            // Lendo o first cluster
    	if(data->fat_type == FAT12 || data->fat_type == FAT16)
    	{
            N = st_directory.DIR_FstClusLO &0xffff;
                         
    	}
    	else if (data->fat_type == FAT32)
    	{
            N = st_directory.DIR_FstClusLO;
            N = N | (st_directory.DIR_FstClusHI << 16);                 
    	} 
    	else
    	{
    	printf("Erro, Volume deve ser ExFAT, FAT type %d",data->fat_type);
    	return 3; //FIXME Volume deve ser ExFAT, erro
    	}
    
    	eof = 1;

    	do{
   
    
    	data->first_sector_of_cluster = ((  N - 2)*bpb.BPB_SecPerClus) + data->first_data_sector;
    	lba_start = data->first_sector_of_cluster &0x7FFFFFFF;

    	//FIXME analiza se entrada é arquivo ou directório
    	if(st_directory.DIR_Attr == FAT_ATTR_DIRECTORY)
    	{ 
            read_sector(directory + offset,lba_start,bpb.BPB_SecPerClus,dev);
            offset_path += fat_path_offset(path0) +1;

            if(count_path == 0){
            dir = 1;

            }
	
	  
        

    	}
    	else if(st_directory.DIR_Attr == FAT_ATTR_ARCHIVE )
    	{ 
        //FIXME aqui!  pegamos o endereço LBA
        // Para a lista vfs

        if(w == 1){
        strcpy(out,"FILE\0"); //Assinatura
        (*(uint32_t*)(out + 4076 + 0)) = bpb.BPB_BytsPerSec;
        (*(uint32_t*)(out + 4080 + 0)) = bpb.BPB_SecPerClus;
        (*(uint32_t*)(out + 4088 + 0)) = st_directory.DIR_FileSize;
        w = 0;
        }

        (*(uint32_t*)(out + 4084 + 0)) += 1;

        (*(uint32_t*)(out + 4096 + out_offset)) = lba_start;
        out_offset +=4;

	
        if(count_path != 0){RET_VAL = -1; goto end;}
	

    	}
    	if (data->fat_type == FAT12)
    	{
            table_value = read_fat(fat_buf,N,dev) &0xFFF;
            EOF = 0xFFF;
            EOF8 =0xFF8; 
    	}
    	else if (data->fat_type == FAT16)
    	{

            table_value = read_fat(fat_buf,N,dev) &0xffff;
            EOF = 0xFFFF;
            EOF8 =0xFFF8;
    	}
    	else if (data->fat_type == FAT32)
    	{           
            table_value = read_fat(fat_buf,N,dev) &0x0FFFFFFF;
            EOF = 0x0FFFFFFF;
            EOF8 =0x0FFFFFF8;

    	}           
    	if ((table_value !=EOF) && (table_value != EOF8)){ 
            N = table_value;
            offset =  offset + (bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus);
    	}
    	else offset = 0;

    	if((table_value ==EOF) || (table_value == EOF8))
        eof =0;

    	}while(eof); 

    	//FIXME
    	if(count_path)
    	{
    	--count_path;
    	goto goto_0;
    	}
    	else{
    	if(dir)fat_read_dir(directory,out,filename); 
    	RET_VAL = 0; goto end;} //SUCCESSFUL

    	} 
    	else
    	{
        // Entrada ainda não encontrada, incrementa ponteiro no goto_8
        //memset(filename,0,strlen (path));
    	}   //final do bloco if((strncmp(name,path,strlen (path))) != 0)

goto_2:   

    	// incrementa ponteiro
    	offset_dir = offset_dir + 32;
    	goto goto_1; 

}//Read
 

end:
    	free(filename);
    	free(path0);
    	free(directory);
    	free(fat_buf);
    	free(data);              
    	return RET_VAL;

}
