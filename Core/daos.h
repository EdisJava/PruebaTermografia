#ifndef DAOS_H
#define DAOS_H

#include "entities.h"
#include <QList>
#include <QString>

class TermogramaDAO {
public:
    TermogramaDAO();
    bool insert(const Termograma& t);
    bool update(const Termograma& t);
    Termograma getById(long id);
    QList<Termograma> getByElementoRed(long idElemento);
    QList<Termograma> getByFiltros(long idCampaña, const QString& circuito);
};

class PuntoCalienteDAO {
public:
    PuntoCalienteDAO();
    bool save(const PuntoCaliente& p, long idTermograma);
    bool remove(long id);
    QList<PuntoCaliente> getByTermograma(long idTermograma);
};

class ElementoRedDAO {
public:
    ElementoRedDAO();
    ElementoRed buscarPorCodigo(const QString& codigo);
    void vincularTermograma(long idElemento, long idTermograma);
};

#endif
