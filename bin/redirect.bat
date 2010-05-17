echo Do NOT run this via "double-click"
echo Run a CMD terminal, and execute it from the FRED (top level) dir.
echo (comment out this warning to continue)
pause
exit

echo rosenfel@repository.psc.edu:/afs/psc.edu/projects/midas/cvsrep > %temp%\Root

copy CVS\Root CVS\Root.backup
copy %temp%\Root CVS\Root
copy CVS\Root CVS\Root.backup
copy %temp%\Root bin\CVS\Root
copy CVS\Root CVS\Root.backup
copy %temp%\Root tests\CVS\Root
copy CVS\Root CVS\Root.backup
copy %temp%\Root tests\vaccine\CVS\Root
copy CVS\Root CVS\Root.backup
copy %temp%\Root tests\vaccine\TEST\CVS\Root
copy CVS\Root CVS\Root.backup
copy %temp%\Root TEST\CVS\Root

del %temp%\Root
