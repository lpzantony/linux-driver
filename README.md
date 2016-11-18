# **TPT35 - Antony Lopez**

## **1. Explorons le noyau Linux**
### Configuration et compilation du noyau
- **QUESTION 1 :** _D'où proviennent les fichiers `/boot/config-*` ? Dans quels cas sont-ils utiles ?_

Les fichiers `/boot/config-*` sont générés par la compilation du noyau. Ils sont présents à titre indicatif si jamais on souhaite recompiler le noyau de l'OS courant.

- **QUESTION 3 :** *Existe-t-il d'autres cibles du Makefile ayant un impact sur la configuration ? Si oui lesquelles et à quoi peuvent-elles servir ?*

Il existe au total une vingtaine  d'autres configurations. La principale différence entre ces configurations repose sur l'outil utilisé pour configurer le noyau (Qt, GTK+ ...).  
Il exitste également des configurations qui valident/ invalident toutes les options, ou même des configurations random pour les plus courageux. (Ou plus sérieusement, pour pouvoir tester la compatibilité entre les options)

### Exploration du noyau

- **QUESTION 3 :** *Calculez la taille de chacun des répertoires à la racine (indice : utilisez la commande `du`). Qu'en pensez-vous ?*

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


### Outils pour l'exploration du code source

Deux sites utilisant LXR sont disponibles pour naviguer dans le code source du noyau Linux :
>https://lxr.missinglinkelectronics.com/linux

>http://lxr.free-electrons.com/source


## **2. Écriture d'un premier module**
### Réflexion
- **QUESTION :** *Quels sont les avantages de l'utilisation d'un module plutôt que du code compilé en dur directement dans l'image du noyau ?*

Faire un module permet de ne charger ses fonctionnalités que lorsqu'elles sont requises. On allège donc le noyau.

### Votre premier module
- **QUESTION :** *Chargez votre module (depuis la carte). Rappel : si vous avez compilé votre module quelque part dans /home/XXX sur le PC, il est visible depuis la carte grâce à SSHFS*

`sudo insmod first.ko`


- **QUESTION :** *Pourquoi ne se passe-t-il rien (en apparence) ?*

Car le code C du module ne comporte pas de `printf()`, mais plutôt des `pr_info()` qui n'affichent rien dans la console. On peut tout de même voir les message apparaître avec la commande `$ dmesg`.

- **QUESTION :** *Déchargez votre module*

`rmmod first`

### Makefile pour la compilation de modules externes
- **QUESTION :** *Lisez la documentation et expliquez comment fonctionne le Makefile présenté plus haut*

Extrait de `Documentation/kbuild/modules.txt` :

>The check for KERNELRELEASE is used to separate the two parts of the makefile. In the example, kbuild will only see the two assignments, whereas "make" will see everything except these two assignments. This is due to two passes made on the file: the first pass is by the "make" instance run on the command line; the second pass is by the kbuild system, which is initiated by the parameterized "make" in the default target.

 Donc le fichier Makefile se découpe en deux parties : celle visible par kbuild (`obj-m  := first.o`) et celle vue par make:

 ```make
 KDIR ?= /lib/modules/`uname -r`/build

 default:
 	$(MAKE) -C $(KDIR) M=$$PWD
```

Pour compiler un module externe, taper la commande:

>make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm KDIR=/home/alopez/linux-socfpga/build/

### Gestionnaires d'interruptions
- **QUESTION :** *Faites un tableau récapitulant ce qu'a le droit de faire ou pas les fonctions utilisées dans chacun des cas : gestionnaire d'interruption classique, __tasklet, workqueue, threaded IRQ__, ainsi que la latence d'exécution des trois derniers mécanismes.*

gestionnaire | Droits du Handler
-------------|-------------------
gestionnaire d'interruption classique (__GIC__)| Pas d'échange vers le userspace, execution la plus rapide possible et pas de fonctions bloquantes
tasklet      | On peut executer des traitements relativement longs mais toujours pas le droit aux fonctions bloquantes. Ce gestionnaire d'interruption s'exécute toujours dans le contexte d'interruption donc assez rapidement après le GIC
workqueue    | Contrairement aux tasklets, le travail effectué par une workqueue est exécuté dans le contexte d'un processus, on peut donc prendre tout le temps dont on a besoin et même appeler des fonctions bloquantes. Une workqueue peut être explicitement retardée avec une valeur de temps exprimée en _jiffies_. Elle est donc possiblement exécutée bien plus tard que le GIC.
threaded IRQ |le gestionnaire threaded IRQ permet de traiter une interruption dans un thread noyau. Les opérations de traitement de l'interruption peuvent alors être bloquantes.

Outil pour afficher les interruptions actives en live:
>watch -n1 "cat /proc/interrupts"
