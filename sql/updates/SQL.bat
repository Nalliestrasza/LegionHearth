TITRE Regroupeur SQL
:menu
echo.
echo Bienvenue dans le Regroupeur de fichiers .sql
echo.
echo Voulez-vous regrouper vos fichiers OUI ou NON
echo Appuyez sur \'O\' ou \'N\'
echo.
set /p note= Votre réponse :
if %note%== o goto sintax.
if %note%== O goto sintax.
if %note%== n goto quit.
if %note%== N goto quit.
echo
sintax.
copy *.sql "SQL Assembler".sql
echo.
 
REM On va mettre la phrase de fin de code (ceci est un commentaire)
 
echo Fichier terminé sous le nom "SQL Assembler".
echo.
pause
exit
:quit.
exit