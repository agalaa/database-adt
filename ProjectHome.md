# Sqlite database ADT #

## Historia ##
Para una materia de la facultad con mi grupo elegimos usar
[SQLite](http://www.sqlite.org/). Preguntando por ahí,
[Juan Martín Sotuyo Dodero](mailto:juansotuyoATgmail) había hecho un ADT, así
que lo tomé prestado y lo adapté para mi trabajo.


Hace poco descubrí un link interesante con código de Marcus Grimm haciendo uso
de SQLite con threads.
[Link](http://www.sqlite.org/cvstrac/wiki?p=SampleCode)


Esto no es otra cosa que alguna que otra mejora al código de Juan y el agregado
de funciones de Marcus Grimm. Los créditos son para ellos dos. :)

## ¿Cómo usar? ##
  * Bajar sqlite-amalgamation de [aquí](http://www.sqlite.org/download.html) y descomprirmir en /sqlite/.
    * Al momento de escribir esto, la última versión es: _sqlite-amalgamation-3\_6\_11.zip_
  * Ver /example/.

## TODO ##
No me maté escribiendo el código. Hay muchas cosas que están mal y deben ser
mejoradas. Dejo acá una lista de cosas que me gustaría agregar.

### Ocultamiento ###
Como verán en el código, tanto la función que agrega usuarios como la que lista
están en databaseADT.c. Eso está mal. Habría que hacer pública la función
**QueryExecute**.

### Archivo de Log ###
  * **La función que loggea no es threadSafe**. Si van a usarlo con threads, deberían modificar el código de la función o no loggear pasándole NULL.

### Transacciones ###
Tanto **BeginTrans** como **EndTrans** están comentadas en el código y son copy
paste del código de Marcus Grimm. Habría que adaptarlas.

### Schema desde archivo ###
El parser es muy simple. Lee hasta que encuentra un ';' y ejecuta.
Puede haber problemas con esto.

### Varios ###
  * Migrar este todo a tickets.
  * MakeFile.
  * Traducir a inglés este README.