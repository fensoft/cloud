#include "myunzip.h"
#include "unzip.h"
#include <iowin32.h>
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
#include <QVariant>
#include <QMessageBox>
#include <QIcon>
#include <QDateTime>

int do_extract_currentfile(unzFile uf,
                           const int* popt_extract_without_path,
                           int* popt_overwrite,
                           const char* password,
                           char* folder,
                           uInt size_buf)
{
    char filename_inzip2[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;

    unz_file_info64 file_info;
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
            FILE* ftestexist;
            ftestexist = fopen64(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
            }
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
  unsigned long long int global_maxsz = 0;
  unsigned long long int global_maxnb = 0;
  unsigned long long int global_currentsz = 0;
  unsigned long long int global_currentnb = 0;

  QDateTime begin = QDateTime::currentDateTime();

  qDebug() << "void MyUnzip::run() - 1" << zip;

  unzFile uf[128];
  unz_global_info64 gi[128];
  unsigned long long int sz[128];

  zlib_filefunc64_def ffunc[128];
  for (int index = 0; index < zip.size(); index++)
  {
    fill_win32_filefunc64A(&ffunc[index]);
    QString test = QCoreApplication::applicationDirPath() + "/" + dest + "/" + location + "/" + zip.at(index);
    qDebug() << test;
    QFileInfo qfi(test);
    sz[index] = qfi.size();
    qDebug() << qfi.size();
    const char* zipfilename = test.toStdString().c_str();
    uf[index] = unzOpen2_64(zipfilename,&ffunc[index]);
    if (uf[index]==NULL)
    {
      qDebug() << "Cannot open " << zipfilename;
      return;
    }

    gi[index].number_entry = 0;
    int err = unzGetGlobalInfo64(uf[index],&gi[index]);
    if (err!=UNZ_OK)
      qDebug() << "error" << err << "with zipfile in unzGetGlobalInfo \n";
  }
  //
  for (int index = 0; index < zip.size(); index++){
    global_maxnb += gi[index].number_entry;
    global_maxsz += sz[index];
    qDebug() << gi[index].number_entry;
  }
  emit max((double)(global_maxsz/1048576ll)+global_maxnb/50);

  for (int index = 0; index < zip.size(); index++)
  {
    //unsigned long long int begin = unzGetLeft(uf[index]);
    qDebug() << "void MyUnzip::run() - 2";
    for (uLong i=0;i<gi[index].number_entry;i++)
    {
      qDebug() << i << "/" << gi[index].number_entry;
      int opt_extract_without_path = 0;
      int opt_overwrite = 1;
      char* password = 0;
      //((unz64_s*)uf[index])->
      global_currentsz+=unzGetLeft(uf[index]);
      global_currentnb++;
      //qDebug() << global_current;
      //qDebug() << QVariant(global_currentsz);
      //qDebug() << QVariant((quint64)global_currentnb);
      emit newState((double)(global_currentsz / 1048576ll)+global_currentnb/50);
      if (do_extract_currentfile(uf[index],&opt_extract_without_path,
                                 &opt_overwrite,
                                 password,
                                 dest.toAscii().data(),
                                 size_buf) != UNZ_OK)
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
  emit(end());

  qDebug() << "end!!!";
  qDebug() << begin.secsTo(QDateTime::currentDateTime());
  //QMessageBox::critical(0, "", "toto");

  return;
}

void MyUnzip::setZip(QString l, QStringList s)
{
  zip = s;
  location = l;
}

void MyUnzip::setDest(QString s)
{
  dest = s;
}

int MyUnzip::state()
{
  return 0;
}

void MyUnzip::setSizeBuf(unsigned int s)
{
  size_buf = s;
}
