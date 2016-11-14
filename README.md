# **TPT35 - Antony Lopez**

## **1. Explorons le noyau Linux**
### Configuration et compilation du noyau
- **QUESTION 1 :** _D'où proviennent les fichiers `/boot/config-*` ? Dans quels cas sont-ils utiles ?_

Les fichiers `/boot/config-*` sont générés par la compilation du noyau. Ils sont présents à titre indicatif si jamais on souhaite recompiler le noyau de l'OS courant.

- **QUESTION 3 :** *Existe-t-il d'autres cibles du Makefile ayant un impact sur la configuration ? Si oui lesquelles et à quoi peuvent-elles servir ?*

Il existe au total une vingtaine  d'autres configurations. La principale différence entre ces configurations repose sur l'outil utilisé pour configurer le noyau (Qt, GTK+ ...).  
Il exitste egalement des configurations qui valident/invalident toutes les options, ou même des configurations random pour les plus courageux.

### Exploration du noyau
- **QUESTION 2 :** *Cherchez à quoi correspondent tous les fichiers et les répertoires à la racine des sources du noyau*



- **QUESTION 3 :** *Calculez la taille de chacun des répertoires à la racine (indice : utilisez la commande du). Qu'en pensez-vous ?*

Avec `$ du -s ./*/` on obtient en octets la taille des dossiers suivants:


arch|block|build|certs|crypto|Documentation|drivers|firmware|fs|include |init
----|-----|-----|-----|------|-------------|-------|--------|--|--------|----
138M|1,2M |1,1G	|28K  |3,1M  |35M          |395M   |6,1M    |37M |35M |192K

ipc|kernel|lib|mm|net|samples|scripts|security|sound|tools|usr|virt
---|------|---|--|---|-------|-------|--------|-----|-----|---|----
248K |7,4M |3,7M |3,3M |27M |648K |3,2M |2,4M |32M |13M |36K |516K

La majorité des dossiers restent en dessous des 40 Mo, sauf le dossier `build` qui atteint 1,1 Go ainsi que le dossier `driver` à 395 Mo.

Le dossier `arch` est le seul qui est spécifique pour chaque architecture. Tous les autres dossiers contiennent du code indépendant de la plateforme.


Deux sites utilisant LXR sont disponibles pour naviguer dans le code source du noyau Linux :


### Outils pour l'exploration du code source

Deux sites utilisant LXR sont disponibles pour naviguer dans le code source du noyau Linux :
>https://lxr.missinglinkelectronics.com/linux

>http://lxr.free-electrons.com/source



### Multiplication and Overflow
- **QUESTION :** *Choose two 5-bi*



## **2. Processor Design**
### Instruction Set Architecture
- **QUESTION :** *Des*



- **QUESTION :** *Provide *

### 2.2 Pipelining
- **QUESTION :** *Draw a*
