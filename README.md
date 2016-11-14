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

```
138M	./arch/  
1,2M	./block/  
1,1G	./build/  
28K     ./certs/  
3,1M	./crypto/  
35M     ./Documentation/  
395M	./drivers/  
6,1M	./firmware/  
37M     ./fs/  
35M     ./include/  
192K	./init/  
248K	./ipc/  
7,4M	./kernel/  
3,7M	./lib/  
3,3M	./mm/  
27M     ./net/  
648K	./samples/  
3,2M	./scripts/  
2,4M	./security/  
32M     ./sound/  
13M     ./tools/  
36K     ./usr/  
516K	./virt/  
```

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
