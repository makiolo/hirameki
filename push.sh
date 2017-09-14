#!/bin/bash

trunk=XXXXXXXX/trunk
branch=XXXXXXXX/branches/dev
ticket=111111
username=XXXXXXXX

count=$(svn diff . | grep "svn:mergeinfo" | wc -l)
if (( count > 0 )); then
	echo no puedes entrar con mergeinfo en "."
	exit 1
fi

svn ls ^/$branch 2>&1 > /dev/null
if (( $? == 0 )); then
	echo skip svn copy, $branch already exists
	svn merge --dry-run --allow-mixed-revisions ^/$trunk ^/$branch
	if (( $? != 0 )); then
		echo no es posible cambiar a $branch
		exit 1
	fi
else
	echo es una nueva rama
	svn copy ^/$trunk ^/$branch -m "#$ticket - create branch" --no-auth-cache --username $username
fi
svn sw ^/$branch
echo -- changed to $branch

count=$(svn diff | wc -l)
if (( count > 0 )); then
	svn commit --no-auth-cache --username $username -m "#$ticket - initial commit"
fi
svn info
