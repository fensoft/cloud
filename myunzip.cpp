#include "myunzip.h"
#include "unzip.h"
#include <iowin32.h>
#define WRITEBUFFERSIZE (8192)
#define WRITEBUFFERSIZE 3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <qdebug.h>
#include <QDir>
#include <QApplication>
#include <QCoreApplication>

int do_extract_currentfile(unzFile uf,
                           const int* popt_extract_without_path,
                           int* popt_overwrite,
                           const char* password,
                           char* folder)
{
    char filename_inzip2[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    //uLong ratio=0;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip2,sizeof(filename_inzip2),NULL,0,NULL,0);
    char* filename_inzip = (char*)malloc(4096);
    filename_inzip[0] = 0;
    strcat(filename_inzip, folder);
    strcat(filename_inzip, "/");
    strcat(filename_inzip, filename_inzip2);
    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            //printf("creating directory: %s\n",filename_inzip);
            _mkdir(filename_inzip);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = fopen64(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char *answer;
                    int ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    //ret = scanf("%1s",answer);
                    ret = 1; answer="A";
                    if (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            fout=fopen64(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                _mkdir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=fopen64(write_filename,"wb");
            }

            if (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        if (fout!=NULL)
        {
            //printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    fclose(fout);

            /*if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);*/
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}

void MyUnzip::run()
{
  unsigned long long int global_max = 0;
  unsigned long long int global_current = 0;
  //QDir::setCurrent("games") ;
  qDebug() << "void MyUnzip::run() - 1" << zip;
  //QString test = QCoreApplication::applicationDirPath() + "/"+zip;
  //qDebug() << QCoreApplication::applicationDirPath();
  //const char* zipfilename = test.toStdString().c_str();
  unzFile uf[128];
  unz_global_info64 gi[128];
  int ret_value=0;

  zlib_filefunc64_def ffunc[128];
  //int i = 0;
  for (int index = 0; index < zip.size(); index++)
  {
    fill_win32_filefunc64A(&ffunc[index]);
    QString test = QCoreApplication::applicationDirPath() + "/" + dest + "/" + zip.at(index);
    const char* zipfilename = test.toStdString().c_str();
    uf[index] = unzOpen2_64(zipfilename,&ffunc[index]);
    if (uf[index]==NULL)
    {
      qDebug() << "Cannot open " << zipfilename;
      return;
    }

    FILE* fout=NULL;

    gi[index].number_entry = 0;
    int err = unzGetGlobalInfo64(uf[index],&gi[index]);
    if (err!=UNZ_OK)
      qDebug() << "error" << err << "with zipfile in unzGetGlobalInfo \n";
  }
  //
  for (int index = 0; index < zip.size(); index++){
    global_max += gi[index].number_entry;
    qDebug() << gi[index].number_entry;
  }
  emit max((double)global_max);
  for (int index = 0; index < zip.size(); index++)
  {
    qDebug() << "void MyUnzip::run() - 2";
    for (uLong i=0;i<gi[index].number_entry;i++)
    {
      //printf("%i out of %i\n", i, gi.number_entry);
      qDebug() << i << "/" << gi[index].number_entry;
      //*global_current = i;
      //*global_max = gi.number_entry;
      int opt_extract_without_path = 0;
      int opt_overwrite = 1;
      char* password = 0;
      global_current++;
      emit newState((double)global_current);
      if (do_extract_currentfile(uf[index],&opt_extract_without_path,
                                 &opt_overwrite,
                                 password,
                                 dest.toAscii().data()) != UNZ_OK)
        break;

      if ((i+1)<gi[index].number_entry)
      {
        int err = unzGoToNextFile(uf[index]);
        if (err!=UNZ_OK)
        {
          printf("error %d with zipfile in unzGoToNextFile\n",err);
          break;
        }
      }
    }
    qDebug() << "void MyUnzip::run() - 3";
    unzClose(uf[index]);
  }
  //QDir::setCurrent("..");
  emit(end());
  return;
}

void MyUnzip::setZip(QStringList s)
{
  zip = s;
}

void MyUnzip::setDest(QString s)
{
  dest = s;
}

int MyUnzip::state()
{
  return 0;
}
