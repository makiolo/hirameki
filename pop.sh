#!/bin/bash

trunk=BBVADevelopmentSupport/trunk
branch=BBVADevelopmentSupport/branches/xe33660b
ticket=12784
username=xe33660
count=$(svn diff | wc -l)
if (( count == 0 )); then
	echo merge changes
	svn merge --dry-run --allow-mixed-revisions ^/$branch ^/$trunk
	if (( $? == 0 )); then
		echo ok
		svn sw ^/$trunk
		svn merge --allow-mixed-revisions ^/$branch
		echo -- returned to $trunk
	else
		echo error retornando la rama, prueba actualizar primero
		exit 1
	fi
else
	svn st
	echo
	echo -- Tienes cambios locales, commitealos antes de volver
	echo
	exit 1
fi
svn info
