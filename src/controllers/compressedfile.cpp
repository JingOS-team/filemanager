#include "compressedfile.h"

#include <KArchive/KZip>
#include <KArchive/KTar>
#include <KArchive/KZip>
#include <KArchive/kcompressiondevice.h>
#include <KArchive/kfilterdev.h>
#include <KArchive/k7zip.h>
#include <KArchive/kar.h>
#include <qdiriterator.h>

CompressedFile::CompressedFile(QObject *parent) : QObject(parent), m_model(new CompressedFileModel(this))
{
}

CompressedFileModel::CompressedFileModel(QObject *parent) : MauiList(parent)
{
}

FMH::MODEL_LIST CompressedFileModel::items() const
{
    return m_list;
}

void CompressedFileModel::setUrl(const QUrl &url)
{
    qDebug() << "@gadominguez File:fm.cpp Funcion: getEntries  Url:" << url.toString();
    emit this->preListChanged();
    m_list.clear();

    KArchive *kArch = CompressedFile::getKArchiveObject(url);
    kArch->open(QIODevice::ReadOnly);
    assert(kArch->isOpen() == true);
    if (kArch->isOpen())
    {
        qDebug() << "@gadominguez File:fm.cpp Funcion: getEntries  Entries:" << kArch->directory()->entries();

        for (auto entry : kArch->directory()->entries())
        {
            auto e = kArch->directory()->entry(entry);

            this->m_list << FMH::MODEL{{FMH::MODEL_KEY::LABEL, e->name()}, {FMH::MODEL_KEY::ICON, e->isDirectory() ? "folder" : FMH::getIconName(e->name())}, {FMH::MODEL_KEY::DATE, e->date().toString()}};
        }
    }

    emit this->postListChanged();
}

void CompressedFile::extract(const QUrl &where, const QString &directory)
{
    if (!m_url.isLocalFile())
        return;

    qDebug() << "@gadominguez File:fm.cpp Funcion: extractFile  "
             << "URL: " << m_url << "WHERE: " << where.toString() << " DIR: " << directory;

    QString where_ = where.toLocalFile() + "/" + directory;

    auto kArch = CompressedFile::getKArchiveObject(m_url);
    kArch->open(QIODevice::ReadOnly);
    qDebug() << "@gadominguez File:fm.cpp Funcion: extractFile  " << kArch->directory()->entries();
    assert(kArch->isOpen() == true);
    if (kArch->isOpen())
    {
        bool recursive = true;
        kArch->directory()->copyTo(where_, recursive);
    }
}

/*
         *
         *  CompressTypeSelected is an integer and has to be acorrding with order in Dialog.qml
         *
         */
bool CompressedFile::compress(const QVariantList &files, const QUrl &where, const QString &fileName, const int &compressTypeSelected)
{
    bool error = true;
    assert(compressTypeSelected >= 0 && compressTypeSelected <= 8);
    for (auto uri : files)
    {
        qDebug() << "@gadominguez File:fm.cpp Funcion: compress  " << QUrl(uri.toString()).toLocalFile() << " " << fileName;

        if (!QFileInfo(QUrl(uri.toString()).toLocalFile()).isDir())
        {
            auto file = QFile(QUrl(uri.toString()).toLocalFile());
            file.open(QIODevice::ReadWrite);
            if (file.isOpen() == true)
            {
                switch (compressTypeSelected)
                {
                case 0: //.ZIP
                {
                    auto kzip = new KZip(QUrl(where.toString() + "/" + fileName + ".zip").toLocalFile());
                    kzip->open(QIODevice::ReadWrite);
                    assert(kzip->isOpen() == true);

                    kzip->writeFile(uri.toString().remove(where.toString(), Qt::CaseSensitivity::CaseSensitive), // Mirror file path in compressed file from current directory
                                    file.readAll(), 0100775, QFileInfo(file).owner(), QFileInfo(file).group(), QDateTime(), QDateTime(), QDateTime());
                    kzip->close();
                    error = false;
                    break;
                }
                case 1: // .TAR
                {
                    auto ktar = new KTar(QUrl(where.toString() + "/" + fileName + ".tar").toLocalFile());
                    ktar->open(QIODevice::ReadWrite);
                    assert(ktar->isOpen() == true);
                    ktar->writeFile(uri.toString().remove(where.toString(), Qt::CaseSensitivity::CaseSensitive), // Mirror file path in compressed file from current directory
                                    file.readAll(), 0100775, QFileInfo(file).owner(), QFileInfo(file).group(), QDateTime(), QDateTime(), QDateTime());
                    ktar->close();
                    error = false;
                    break;
                }
                case 2: //.7ZIP
                {
                    //TODO: KArchive no permite comprimir ficheros del mismo modo que con TAR o ZIP. Hay que hacerlo de otra forma y requiere disponer de una libreria actualizada de KArchive.
                    auto k7zip = new K7Zip(QUrl(where.toString() + "/" + fileName + ".7z").toLocalFile());
                    k7zip->open(QIODevice::ReadWrite);
                    assert(k7zip->isOpen() == true);
                    k7zip->writeFile(uri.toString().remove(where.toString(), Qt::CaseSensitivity::CaseSensitive), // Mirror file path in compressed file from current directory
                                     file.readAll(), 0100775, QFileInfo(file).owner(), QFileInfo(file).group(), QDateTime(), QDateTime(), QDateTime());
                    k7zip->close();
                    error = false;
                    break;
                }
                case 3: //.AR
                {
                    //TODO: KArchive no permite comprimir ficheros del mismo modo que con TAR o ZIP. Hay que hacerlo de otra forma y requiere disponer de una libreria actualizada de KArchive.
                    auto kar = new KAr(QUrl(where.toString() + "/" + fileName + ".ar").toLocalFile());
                    kar->open(QIODevice::ReadWrite);
                    assert(kar->isOpen() == true);
                    kar->writeFile(uri.toString().remove(where.toString(), Qt::CaseSensitivity::CaseSensitive), // Mirror file path in compressed file from current directory
                                   file.readAll(), 0100775, QFileInfo(file).owner(), QFileInfo(file).group(), QDateTime(), QDateTime(), QDateTime());
                    kar->close();
                    error = false;
                    break;
                }
                default:
                    qDebug() << "ERROR. COMPRESSED TYPE SELECTED NOT COMPATIBLE";
                    break;
                }
            }
            else
            {
                qDebug() << "ERROR. CURRENT USER DOES NOT HAVE PEMRISSION FOR WRITE IN THE CURRENT DIRECTORY.";
                error = true;
            }
        }
        else
        {
            qDebug() << "Dir: " << QUrl(uri.toString()).toLocalFile();
            auto dir = QDirIterator(QUrl(uri.toString()).toLocalFile(), QDirIterator::Subdirectories);
            auto file = QFile(QUrl(uri.toString()).toLocalFile());
            file.open(QIODevice::ReadWrite);
            auto kzip = new KZip(QUrl(where.toString() + "/" + fileName + ".zip").toLocalFile());
            kzip->open(QIODevice::ReadWrite);
            assert(kzip->isOpen() == true);

            while (dir.hasNext())
            {
                auto entrie = dir.next();
                qDebug() << entrie << " " << where.toString() << QFileInfo(entrie).isFile();
                if (QFileInfo(entrie).isFile() == true)
                {
                    qDebug() << entrie.remove(QUrl(where).toLocalFile(), Qt::CaseSensitivity::CaseSensitive);
                    kzip->writeFile(entrie.remove(where.toString(), Qt::CaseSensitivity::CaseSensitive), // Mirror file path in compressed file from current directory
                                    file.readAll(), 0100775, QFileInfo(file).owner(), QFileInfo(file).group(), QDateTime(), QDateTime(), QDateTime());
                }
            }

            kzip->close();
        }
    }

    //kzip->prepareWriting("Hello00000.txt", "gabridc", "gabridc", 1024, 0100777, QDateTime(), QDateTime(), QDateTime());
    //kzip->writeData("Hello", sizeof("Hello"));
    //kzip->finishingWriting();

    return error;
}

KArchive *CompressedFile::getKArchiveObject(const QUrl &url)
{
    KArchive *kArch = nullptr;

    /*
                        * This checks depends on type COMPRESSED_MIMETYPES in file fmh.h
                        */
    qDebug() << "@gadominguez File: fmstatic.cpp Func: getKArchiveObject MimeType: " << FMH::getMime(url);

    if (FMH::getMime(url).contains("application/x-tar"))
    {
        kArch = new KTar(url.toString().split(QString("file://"))[1]);
    }
    else if (FMH::getMime(url).contains("application/zip"))
    {
        kArch = new KZip(url.toString().split(QString("file://"))[1]);
    }
    else if (FMH::getMime(url).contains("application/x-archive"))
    {
        kArch = new KAr(url.toString().split(QString("file://"))[1]);
    }
    else if (FMH::getMime(url).contains("application/x-7z-compressed"))
    {
        kArch = new K7Zip(url.toString().split(QString("file://"))[1]);
    }
    else
    {
        qDebug() << "ERROR. COMPRESSED FILE TYPE UNKOWN " << url.toString();
    }

    return kArch;
}

void CompressedFile::setUrl(const QUrl &url)
{
    if (m_url == url)
        return;

    m_url = url;
    emit this->urlChanged();

    m_model->setUrl(m_url);
}

QUrl CompressedFile::url() const
{
    return m_url;
}

CompressedFileModel *CompressedFile::model() const
{
    return m_model;
}