#include "daos.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <QDebug>

TermogramaDAO::TermogramaDAO() {}

bool TermogramaDAO::insert(const Termograma& t) {
    QSqlQuery query;
    query.prepare("INSERT INTO termogramas (nombre_archivo, ruta_fisica, fecha, x, y, z, ancho, alto) "
                  "VALUES (:nombre, :ruta, :fecha, :x, :y, :z, :w, :h)");
    query.bindValue(":nombre", t.nombreArchivo);
    query.bindValue(":ruta", t.rutaFisica);
    query.bindValue(":fecha", t.fechaCaptura);
    query.bindValue(":x", t.coordX);
    query.bindValue(":y", t.coordY);
    query.bindValue(":z", t.coordZ);
    query.bindValue(":w", t.ancho);
    query.bindValue(":h", t.alto);

    if (!query.exec()) {
        qDebug() << "Error insertando termograma:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TermogramaDAO::update(const Termograma& t) {
    QSqlQuery query;
    query.prepare("UPDATE termogramas SET x=:x, y=:y, z=:z WHERE id=:id");
    query.bindValue(":x", t.coordX);
    query.bindValue(":y", t.coordY);
    query.bindValue(":z", t.coordZ);
    query.bindValue(":id", (long long)t.id);
    return query.exec();
}

Termograma TermogramaDAO::getById(long id) {
    Termograma t;
    QSqlQuery query;
    query.prepare("SELECT * FROM termogramas WHERE id = :id");
    query.bindValue(":id", (long long)id);

    if (query.exec() && query.next()) {
        t.id = query.value("id").toLongLong();
        t.nombreArchivo = query.value("nombre_archivo").toString();
        t.rutaFisica = query.value("ruta_fisica").toString();
        t.fechaCaptura = query.value("fecha").toDateTime();
        t.coordX = query.value("x").toDouble();
        t.coordY = query.value("y").toDouble();
        t.coordZ = query.value("z").toDouble();
        t.ancho = query.value("ancho").toInt();
        t.alto = query.value("alto").toInt();
    }
    return t;
}

QList<Termograma> TermogramaDAO::getByElementoRed(long idElemento) {
    QList<Termograma> lista;
    QSqlQuery query;
    query.prepare("SELECT t.* FROM termogramas t "
                  "JOIN rel_elemento_termograma r ON t.id = r.id_termograma "
                  "WHERE r.id_elemento = :id");
    query.bindValue(":id", (long long)idElemento);

    if (query.exec()) {
        while (query.next()) {
            Termograma t;
            t.id = query.value("id").toLongLong();
            t.nombreArchivo = query.value("nombre_archivo").toString();
            lista.append(t);
        }
    }
    return lista;
}

QList<Termograma> TermogramaDAO::getByFiltros(long idCampaña, const QString& circuito) {
    Q_UNUSED(idCampaña); Q_UNUSED(circuito);
    return QList<Termograma>(); // Implementar según estructura de tablas de campaña
}

PuntoCalienteDAO::PuntoCalienteDAO() {}

bool PuntoCalienteDAO::save(const PuntoCaliente& p, long idTermograma) {
    QSqlQuery query;
    // Lógica UPSERT: Si el ID es -1 insertamos, si no, actualizamos
    if (p.id == -1) {
        query.prepare("INSERT INTO puntos_calientes (id_termograma, temp_max, x, y, tipo, comentario) "
                      "VALUES (:id_t, :temp, :x, :y, :tipo, :txt)");
    } else {
        query.prepare("UPDATE puntos_calientes SET temp_max=:temp, comentario=:txt, validado=:val "
                      "WHERE id=:id");
        query.bindValue(":id", (long long)p.id);
        query.bindValue(":val", p.validado);
    }

    query.bindValue(":id_t", (long long)idTermograma);
    query.bindValue(":temp", p.temperaturaMax);
    query.bindValue(":x", p.pixelX);
    query.bindValue(":y", p.pixelY);
    query.bindValue(":tipo", (int)p.tipo);
    query.bindValue(":txt", p.comentario);

    return query.exec();
}

bool PuntoCalienteDAO::remove(long id) {
    QSqlQuery query;
    query.prepare("DELETE FROM puntos_calientes WHERE id = :id");
    query.bindValue(":id", (long long)id);
    return query.exec();
}

QList<PuntoCaliente> PuntoCalienteDAO::getByTermograma(long idTermograma) {
    QList<PuntoCaliente> lista;
    QSqlQuery query;
    query.prepare("SELECT * FROM puntos_calientes WHERE id_termograma = :id_t");
    query.bindValue(":id_t", (long long)idTermograma);

    if (query.exec()) {
        while (query.next()) {
            PuntoCaliente p;
            p.id = query.value("id").toLongLong();
            p.temperaturaMax = query.value("temp_max").toFloat();
            p.pixelX = query.value("x").toInt();
            p.pixelY = query.value("y").toInt();
            p.comentario = query.value("comentario").toString();
            lista.append(p);
        }
    }
    return lista;
}

ElementoRedDAO::ElementoRedDAO() {}

ElementoRed ElementoRedDAO::buscarPorCodigo(const QString& codigo) {
    ElementoRed e;
    QSqlQuery query;
    query.prepare("SELECT * FROM elementos_red WHERE codigo = :cod");
    query.bindValue(":cod", codigo);

    if (query.exec() && query.next()) {
        e.id = query.value("id").toLongLong();
        e.codigo = query.value("codigo").toString();
        e.tipo = static_cast<TipoElemento>(query.value("tipo").toInt());
    }
    return e;
}

void ElementoRedDAO::vincularTermograma(long idElemento, long idTermograma) {
    QSqlQuery query;
    query.prepare("INSERT INTO rel_elemento_termograma (id_elemento, id_termograma) "
                  "VALUES (:id_e, :id_t)");
    query.bindValue(":id_e", (long long)idElemento);
    query.bindValue(":id_t", (long long)idTermograma);
    query.exec();
}
