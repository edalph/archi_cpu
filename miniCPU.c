/*
 * -------------------------- TP du module Archi1 -------------------------
 *
 * ATTENTION : un outil de détection de plagiat logiciel sera utilisé lors de la correction,
 * vous avez donc tout intérêt à effectuer un travail PERSONNEL
 *
 * Un mot/registre de 10 à 30 bits est représenté par un tableau d'entiers égaux à 0 ou 1.
 * Une ALSU est représentée par une structure ALSU, avec registre accumulateur
 * et registre d'état.
 * Un CPU (très très simplifié) est représenté par une ALSU et quelques registres
 * nécessaires pour stocker les résultats intermédiaires.
 *
 * Certaines fonctions vous sont fournies, d'autres sont à implanter ou à compléter,
 * de préférence dans l'ordre où eles sont indiquées.
 * Il vous est fortement conseillé de lire attentivement l'ensemble des commentaires.
 *
 * Parmi les opérations arithmétiques et logiques, seules 4 opérations de base sont
 * directement fournies par l'ALSU, les autres doivent être décrites comme des algorithmes
 * travaillant à l'aide des opérateurs de base de l'ALSU simplifiée et pouvant utiliser les 
 * registres du CPU.
 *
 * La fonction main() vous permet de tester au fur et à mesure les fonctions que vous implantez.
 *
 * ----------------------------------------------------------------------------------------------
 *
 * author: B. Girau
 * version: 2016-2017
 */
#include <stdio.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////
// définition de types
/////////////////////////////////////////////////////////

typedef struct {
  int size;
  int *word;
} Register;

typedef struct {
  Register accu;
  int* flags; // indicateurs Z C O N
} ALSU;

/*
  flags[0] <=> Z <=> zero <=> vaut 1 si resultat nul
  flags[1] <=> C <=> carry <=> valeur de la derniere retenue
  flags[2] <=> O <=> overflow <=> vaut 1 si depassement de capacite
  flags[3] <=> N <=> negative <=> vaut 1 si resultat negatif
*/

typedef struct {
  ALSU alsu;
  Register R0;
  Register R1;
  Register R2;
} CPU;

/////////////////////////////////////////////////////////
// fonctions d'initialisation
/////////////////////////////////////////////////////////

/*
 * allocation d'un mot/registre entier de 10 à 30 bits initialisé à 0
 */
Register reg(int size) {
  Register R;
  R.size = size;
  R.word = (int*)malloc(size*sizeof(int));
  return R;
}

/*
 * Initialisation du mot (mot de 10 à 30 bits, codant un entier en Cà2) avec une valeur entière.
 */
void setValue(Register R,int n) {
  int i,invers;
  if (n>=0){
    for(i=0;i<R.size-1;i++){
      R.word[i] = n%2;
      n = n/2;
    }
    R.word[R.size-1] = 0;
  }else{
    n = -n;
    for(i=0;i<R.size-1;i++){
      R.word[i] = n%2;
      n = n/2;
    }
    R.word[R.size-1] = 0;
    invers = 0;
    for (i=0;i<R.size;i++){
      if(invers == 0){
	if(R.word[i] == 1){
	  invers = 1;
	}
      }else{
	if(R.word[i] == 0){
	  R.word[i] = 1;
	}else{
	  R.word[i] = 0;
	}
      }
    }
  }
}

/*
 * instanciation d'un mot de 10 à 30 bits initialisé avec la valeur n
 */
Register initR(int size,int n) {
  Register R=reg(size);
  setValue(R,n) ;
  return R;
}
    
/*
 * Initialisation du mot (mot de 10 à 30 bits) par recopie des bits du mot en paramètre.
 */
void copyValue(Register *R,Register src) {
  int i;
  for(i=0;i<src.size;i++){
    R->word[i]=src.word[i];
  }
}

/*
 * instanciation d'un mot de 10 à 30 bits initialisé par recopie d'un mot
 */
Register copyRegister(Register R) {
  Register C=reg(R.size);
  copyValue(&C,R);
  return C;
}

/*
 * initialise l'ALSU
 */
ALSU initALSU(int size) {
  ALSU res;
  res.accu=reg(size);
  res.flags=(int*)malloc(4*sizeof(int));
  return res;
}

/*
 * initialise le CPU
 */
CPU initCPU(int size) {
  CPU res;
  res.alsu=initALSU(size);
  res.R0=reg(size);
  res.R1=reg(size);
  res.R2=reg(size);
  return res;
}

/////////////////////////////////////////////////////////
// fonctions de lecture
/////////////////////////////////////////////////////////

/*
 * Retourne la valeur entière signée représentée par le mot (complément à 2).
 */    
int intValue(Register R) {
  int res=0,i,puiss=1;
  for(i=0;i<R.size-1;i++){
    res = res+R.word[i]*puiss;
    puiss = puiss*2;
  }
  res = res-R.word[R.size-1]*puiss;
  return res;
}

/*
 * Retourne une chaîne de caractères décrivant les 10 à 30 bits
 */
char* toString(Register R) {
  int i,j=0;
  char* s=(char*)malloc(40*sizeof(char));
  for (i=R.size-1;i>=0;i--) {
    if (R.word[i]==1) s[j]='1';
    else s[j]='0';
    j++;
  }
  return s;
}

/*
 * Retourne l'écriture des indicateurs associés à l'ALSU.
 */
char* flagsToString(ALSU alsu) {
  char *string=(char*)malloc(10*sizeof(char));
  sprintf(string,"z%dc%do%dn%d",alsu.flags[0],alsu.flags[1],alsu.flags[2],alsu.flags[3]);
  return string;
}

/*
 * affiche à l'écran le contenu d'une ALSU
 */
void printing(ALSU alsu) {
  printf("Contenu du registre accumulateur : %s\nEtat des indicateurs : %s\n",toString(alsu.accu),flagsToString(alsu));
}

/////////////////////////////////////////////////////////
// fonctions de manipulations élémentaires
/////////////////////////////////////////////////////////

/*
 * Positionne l'indicateur Z en fonction de l'état de l'accumulateur
 */
void setZ(ALSU *alsu) {
  int nul=1,i=0;
  while (nul && i<alsu->accu.size){
    if(alsu->accu.word[i] == 1){
      nul = 0;
    }
    i++;
  }
  alsu->flags[0] = nul;
}

/*
 * Positionne l'indicateur N en fonction de l'état de l'accumulateur
 */
void setN(ALSU *alsu){
  if(alsu->accu.word[alsu->accu.size-1] == 1){
    alsu->flags[3] = 1;
  }else{
    alsu->flags[3] = 0;
  }
}

/////////////////////////////////////////////////////////
// opérateurs de base de l'ALSU
/////////////////////////////////////////////////////////

/*
 * Stocke le paramètre dans le registre accumulateur
 */
void pass(ALSU *alsu,Register B) {
  int i;
  for(i=0;i<alsu->accu.size;i++){
    alsu->accu.word[i] = B.word[i];
  }
  setZ(alsu);                                 //MaJ indicateurs
  setN(alsu);
}

/*
 * Effectue un NAND (NON-ET) entre le contenu de l'accumulateur et le paramètre.
 */
void nand(ALSU *alsu,Register B) {
  int i;
  for(i=0;i<alsu->accu.size;i++){
    if(alsu->accu.word[i] == 1 && B.word[i] == 1){
      alsu->accu.word[i] = 0;
    }else{
      alsu->accu.word[i] = 1;
    }
  }
  setZ(alsu);                                 //MaJ indicateurs
  setN(alsu);
}


/*
 * Décale le contenu de l'accumulateur de 1 bit vers la droite
 */
void shift(ALSU *alsu) {
  int i;
  for(i=0;i<alsu->accu.size-1;i++){
    alsu->accu.word[i] = alsu->accu.word[i+1];
  }
  alsu->accu.word[alsu->accu.size-1] = 0;
  setZ(alsu);                                 //MaJ indicateurs
  setN(alsu);

}

/*
 * module Full Adder : a+b+cin = s + 2 cout
 * retourne un tableau contenant s et cout
 */
int* fullAdder(int a,int b,int cin) {
  int* res=(int*)malloc(2*sizeof(int));
  if (a == 1 && b == 1){
    res[0] = cin;
    res[1] = 1;
  }else{
    if(a == 0 && b == 0){
      res[0] = cin;
      res[1] = 0;
    }else{
      if (cin == 1){
	res[0] = 0;
      }else{
	res[0] = 1;
      }
      res[1] = cin;
    }
  }
  return res;
}

/*
 * Additionne le paramètre au contenu de l'accumulateur (addition entière Cà2).
 * Les indicateurs sont positionnés conformément au résultat de l'opération.
 */
void add(ALSU *alsu,Register B) {
  int i,cn,an,bn;
  int* res_FA=(int*)malloc(2*sizeof(int));
  res_FA[1]=0;
  for (i=0;i<B.size;i++){
    if (i == B.size-1){          //recuperation des dernieres valeurs des registres pour verifier l'overflow
      cn = res_FA[1];
      an = alsu->accu.word[i];
      bn = B.word[i];
    }
    res_FA = fullAdder(alsu->accu.word[i],B.word[i],res_FA[1]);
    alsu->accu.word[i] = res_FA[0];
  }
  setZ(alsu);                                 //MaJ indicateurs
  setN(alsu);
  alsu->flags[1] = res_FA[1];                 //Carry

  if (an == bn && an != cn){                  //Overflow
    alsu->flags[2] = 1;
  }else{
    alsu->flags[2] = 0;
  }
}

////////////////////////////////////////////////////////////////////
// Opérations logiques :
////////////////////////////////////////////////////////////////////

/*
 * Négation.  NOT(A) = NAND(A,A)
 */
void not(CPU *cpu){
  cpu->R0 = copyRegister(cpu->alsu.accu);
  nand(&cpu->alsu,cpu->R0);
}

/*
 * Et.  AND(A,B) = NAND(NAND(A,B),NAND(A,B))
 */
void and(CPU *cpu,Register B) {
  nand(&cpu->alsu,B);
  cpu->R0 = copyRegister(cpu->alsu.accu);
  nand(&cpu->alsu,cpu->R0);
}


/*
 * Ou.     OR(A,B) = NAND(NOT(A),NOT(B)) = NAND(NAND(A,A),NAND(B,B))
 */
void or(CPU *cpu,Register B) {
  nand(&cpu->alsu,cpu->alsu.accu);   //NOT A
  cpu->R0 = copyRegister(cpu->alsu.accu);
  pass(&cpu->alsu,B);
  nand(&cpu->alsu,cpu->alsu.accu);   //NOT B
  nand(&cpu->alsu,cpu->R0);
}

/*
 * Xor.     XOR(A,B) = NAND(NAND(A,NOT(B)),NAND(NOT(A),B)) = NAND(NAND(A,NAND(B,B)),NAND(NAND(A,A),B))
 */
void xor(CPU *cpu,Register B) {
  cpu->R0 = copyRegister(cpu->alsu.accu); //R0<-A
  nand(&cpu->alsu,cpu->alsu.accu);   //NOT A
  nand(&cpu->alsu,B);
  cpu->R1 = copyRegister(cpu->alsu.accu); //R1<-NAND(NOT(A),B)
  pass(&cpu->alsu,B);
  nand(&cpu->alsu,cpu->alsu.accu);   //NOT B
  nand(&cpu->alsu,cpu->R0);         //NAND(NOT(B),A)
  nand(&cpu->alsu,cpu->R1);        //NAND(NAND(NOT(B),A),NAND(NOT(A),B))
}

/*
 * Décale le receveur de |n| positions.
 * Le décalage s'effectue vers la gauche si n>0 vers la droite dans le cas contraire.
 * C'est un décalage logique (pas de report du bit de signe dans les positions 
 * libérées en cas de décalage à droite).
 * Les indicateurs sont positionnés avec le dernier bit "perdu".
 */
void logicalShift(CPU *cpu,int n) {
  int i;
  if (n>0){
    for (i=0;i<n;i++){
      cpu->R0 = copyRegister(cpu->alsu.accu);
      add(&cpu->alsu,cpu->R0);
    }
  }else{
    for (i=0;i>n;i--){
      shift(&cpu->alsu);
    }
  }
}

/////////////////////////////////////////////////////////
// Opérations arithmétiques entières
/////////////////////////////////////////////////////////

/*
 * Opposé.   -A = NOT(A)+1
 */
void opp(CPU *cpu) {
  cpu->R0 = copyRegister(cpu->alsu.accu);
  nand(&cpu->alsu,cpu->R0);
  setValue(cpu->R1,1);
  add(&cpu->alsu,cpu->R1);
}

/*
 * Soustraction.   A-B = A+(-B)
 */
void sub(CPU *cpu,Register B) {
  cpu->R2 = copyRegister(cpu->alsu.accu);
  pass(&cpu->alsu,B);
  opp(cpu);
  add(&cpu->alsu,cpu->R2);
}

/*
 * Multiplication.
 */
void mul(CPU *cpu,Register B) {
  int i;
  cpu->R1 = copyRegister(cpu->alsu.accu);
  setValue(cpu->alsu.accu,0);
  for (i=0;i<cpu->alsu.accu.size;i++){
    if (B.word[i] == 0){     //si un bit du premier nombre vaut 0, on effectue juste un decalage a droite sur le deuxieme nombre
      cpu->R2 = copyRegister(cpu->alsu.accu);
      pass(&cpu->alsu,cpu->R1);
      logicalShift(cpu,1);
      cpu->R1 = copyRegister(cpu->alsu.accu);
      pass(&cpu->alsu,cpu->R2);
    }else{
      add(&cpu->alsu,cpu->R1);
      cpu->R2 = copyRegister(cpu->alsu.accu);
      pass(&cpu->alsu,cpu->R1);
      logicalShift(cpu,1);
      cpu->R1 = copyRegister(cpu->alsu.accu);
      pass(&cpu->alsu,cpu->R2);
    }
  }
}

/////////////////////////////////////////////////////////
// Programme de test
/////////////////////////////////////////////////////////

int main(int argc,char *argv[]) {
  
  // Champs privés
  
  Register operand,R;
  ALSU alsu;
  CPU cpu;
  
  int chosenInt,integer ;
  int go_on = 1 ;
  
  char* menu =     
    "              Programme de test\n\n0  Quitter\n1  setValue(operande,int)\n2  pass(alsu,operande)\n3  printing(alsu)\n4  afficher toString(operande)\n5  afficher intValue(operande)\n6  afficher intValue(accu)\n7  accu=add(accu,operande)\n8  accu=nand(accu,operande)\n9  accu=sub(accu,operande)\n10 accu=and(accu,operande)\n11 accu=or(accu,operande)\n12 accu=xor(accu,operande)\n13 accu=not(accu)\n14 accu=opp(accu)\n15 accu=logicalShift(accu,int)\n16 accu=mul(accu,operande)\n\n" ;
  
  char* invite = "--> Quel est votre choix  ? " ;
  
  printf("%s",menu) ; 

  operand=reg(20);
  R = reg(20);
  cpu=initCPU(20);
  alsu=cpu.alsu;
  
  while (go_on==1) {
    printf("%s",invite);
    scanf("%d",&chosenInt);
    switch (chosenInt) {
    case 0 : 
      go_on = 0 ;
      break ;
    case 1 :
      printf("Entrez un nombre :"); 
      scanf("%d",&integer);
      setValue(operand,integer);
      R = copyRegister(operand);
      break ;
    case 2 : 
      pass(&alsu,operand);
      break ;
    case 3 : 
      printing(alsu);
      break ;
    case 4 : 
      printf("%s\n",toString(R));
      break ;
    case 5 : 
      printf("intValue(operand)=%d\n",intValue(operand));
      break ;
    case 6 : 
      printf("intValue(accu)=%d\n",intValue(alsu.accu));
      break ;
    case 7 : 
      add(&alsu,operand);
      printf("apres add(), accu = ");
      printing(alsu);
      break ;
    case 8 :
      nand(&alsu,operand);
      printf("apres nand(),accu = ");
      printing(alsu);
      break;
    case 9 : 
      sub(&cpu,operand);
      printf("apres sub(), A = ");
      printing(alsu);
      break ;
    case 10 : 
      and(&cpu,operand);
      printf("apres and(), A = ");
      printing(alsu);
      break ;
    case 11 : 
      or(&cpu,operand);
      printf("apres or(), A = ");
      printing(alsu);
      break ;
    case 12 : 
      xor(&cpu,operand);
      printf("apres xor(), A = ");
      printing(alsu);
      break ;
    case 13 : 
      not(&cpu);
      printf("apres not(), A = ");
      printing(alsu);
      break ;
    case 14 : 
      opp(&cpu);
      printf("apres opp(), A = ");
      printing(alsu);
      break ;
    case 15 : 
      printf("Entrez un entier :") ;
      scanf("%d",&integer);
      logicalShift(&cpu,integer);
      //shift(&cpu.alsu);
      printf("apres logicalshift(), A = ");
      printing(alsu);
      break ;
    case 16 : 
      mul(&cpu,operand);
      printf("apres mul(), A = ");
      printing(alsu);
      break ;
    default : 
      printf("Choix inexistant !!!!\n");
      printf("%s",menu);
    }
  }
  printf("Au revoir et a bientot\n");
  return 0;
}
