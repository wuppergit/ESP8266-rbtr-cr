const char motorHTML_Seite1[] = R"motorHTML1(
<!DOCTYPE html>
<html lang="de">
  <head>
    <meta charset="utf-8" />  
    <link rel="shortcut icon" href="favicon-96x96.png"/>
    <title>Robot-Auto-Steuerung</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <style>   
      body {                           /* anders z.B. div.center */ 
        text-align: center;            /* zentrierte Ausgabe */
      }

      /* fuer Balken und Anzeige*/
      #progressbar {                   /* Balken */
        background-color: #ddd;  
        padding: 3px;                  /* (Höhe des inneren div-Tags) / 2 + padding */
      }
      #progressbar>div {               /* Anzeige */
        background-color: orange; )motorHTML1";

 /*       width:  30%;                     adjust with program */          

const char motorHTML_Seite2[] = R"motorHTML2(
        height: 20px;  
      }     

      /* Schriftgroesse, Umrandung der Schaltflaechen Geschwindigkit */
      #speedContainer { 
        width:  100%;
        height: 50px;   
      }
      .alleSpeed a {                   /* bezieht sich auf a-Tag */
        font-size: 22px;               /* Schriftgrösse der Rechteck-Schaltflaechen */
        border: 2px solid black;       /* Rahmenbreite und -farbe */
        background-color: #f44336;     /* Hintergrundfarbe der Rechteckschaltflächen */
        color: #ffffff;                /* Schriftfarbe */
        border-color: #f44336;         /* Farbe des Rahmens ändern */     
      }   
      .speedFlaeche  {
        border: none;                  /* kein Rand (keine Auswirkung ?)*/
        display: inline-block;         /* stellt das Element als Block dar (wie (<p>) */
        padding: 8px 16px;             /* Abstand Schrift zum Rand */
        width:120px;                   /* Elementbreite 120px */  
        vertical-align: middle;        /* vertikale Ausrichtung eines Elements */
        overflow: hidden;              /* verhalten, wenn der Inhalt ein Box-Element überlappt */
        text-decoration: none;         /* kein Unter-, Durchstreichen o.ä. des Textes */
        color: inherit;                /* erbt die Eigenschaft von "Eltern" (hier keine Auswirkung) */
        background-color: inherit;     /* erbt die Eigenschaft von "Eltern" (hier keine Auswirkung) *//
        text-align: center;            /* horizontale Textausrichtung (Unterstreichung entfällt?!) */
        cursor: pointer;               /* Cursor-Aussehen wenn er auf einem Auswahlfeld ist (hier Handzeiger) */
        white-space: nowrap;           /* Leerzeichenbehandlung bei Zeilenumbruch (hier keine Auswirkung) */
      }
      .langsam {
        float: left!important;         /* überschreibt Voreinstellung, schiebt das nach links*/
      } 
      .schnell {
        float: right!important;        /* überschreibt Voreinstellung, schiebt das nach rechts*/
      } 

      /* Panel */     
      /* Container fuer alle Panelschaltflaechen */
      .panelContainer {
        background-color: transparent; /* Hintergrundfarbe der Rechteckschaltflächen */       
        border: 5px solid black;       /* Rahmenbreite und -farbe */
        border-color: transparent;     /* Farbe des Rahmens ändern */ 
        padding-top: 100px;       
      }
      .panelContainerZeile {
        font-size: 22px;               /* Schriftgrösse */
        color: #ffffff;                /* Schriftfarbe */     
        background-color: transparent; /* Hintergrundfarbe des Containers */        
        border: 8px solid transparent; /* Rahmenbreite, linienart und -farbe */ 
        display: flex;                 /* zeigt ein Element in einem block-level flex Container */
        justify-content: center;       /* Elemente werden in der Mitte des Containers positioniert */   
        text-align: center;            /*  keine Auswrikung zentrierte Ausgabe */         
        white-space: pre-wrap;         /* Leerzeichenbehandlung bei Zeilenumbruch (hier keine Auswirkung) */        
      }     
      /* Container fuer eine Panelschaltflaeche (Parent-Element) */
      .panelParent {
        position: relative; 
        background-color: transparent; /* Hintergrundfarbe der Rechteckschaltflächen */       
        border: 9px solid transparent; /* Rahmenbreite und -farbe */
        width:160px;                   /* Elementbreite 120px */  
        height:160px;                  /* Elementbreite 120px */
        white-space: normal;           /* Leerzeichenbehandlung zurücksetzen */
      }           
      /* Child-Elemente */      
      /* Position der Panel-Schaltflaechen */     
      .panelChild1,  #panelChild2S,
      #panelChild2V, #panelChild2H, #panelChild2L, #panelChild2R {
        position: absolute;             
      }     
      .panelChild1  {                  /* fuer Schrift und Link */
        border: 10px solid transparent;      
        background-color: transparent;     
        z-index: 2;       
      }       
      .panelChild1-href  {
        border: none;                  /* kein Rand (keine Auswirkung ?)*/
        display: inline-block;         /* stellt das Element als Block dar (wie (<p>) */
        padding-top: 60px;             /* Abstand Schrift zum oberen Rand */  
        width:140px;                   /* Breite Inhalt  */ 
        height:80px;                   /* Höhe Inhal */       
        vertical-align: middle;        /* vertikale Ausrichtung eines Elements */
        overflow: hidden;              /* verhalten, wenn der Inhalt ein Box-Element überlappt */
        text-decoration: none;         /* kein Unter-, Durchstreichen o.ä. des Textes */
        color: inherit;                /* erbt die Eigenschaft von "Eltern" (hier keine Auswirkung) */
        background-color: inherit;     /* erbt die Eigenschaft von "Eltern" (hier keine Auswirkung) *//
        text-align: center;            /* horizontale Textausrichtung (Unterstreichung entfällt?!) */
        cursor: pointer;               /* Cursor-Aussehen wenn er auf einem Auswahlfeld ist (hier Handzeiger) */
        }     
      
      /* Panel-Schaltflaechen Richtung */     
      #panelChild2S, #panelChild2V, #panelChild2H, #panelChild2L, #panelChild2R {
        width: 0;
        height: 0;  
        background-color: transparent; /* rot fuer Debug */           
        border-style: solid;        
        z-index: 1;
      }
      #panelChild2S { 
        border-width: 80px;            /* alle Randbreiten */
        border-color: #35b19b ;        /* Randfarbee */
      }
      #panelChild2V { 
        border-width: 0px 80px 160px 80px;  /* Randbreite oben, rechts, unten, links */
        border-color: transparent transparent #35b19b transparent;  /* Randfarbee oben, rechts, unten, links */
      }
      #panelChild2H { 
        border-width: 160px 80px 0px 80px;  /* Randbreite oben, rechts, unten, links */
        border-color: #35b19b transparent transparent transparent;  /* Randfarbee oben, rechts, unten, links */
      }
      #panelChild2L{  
        border-width: 80px 160px 80px 0px;  /* Randbreite oben, rechts, unten, links */
        border-color: transparent #35b19b transparent transparent;  /* Randfarbee oben, rechts, unten, links */
      }
      #panelChild2R { 
        border-width: 80px 0px 80px 160px;  /* Randbreite oben, rechts, unten, links */
        border-color: transparent transparent transparent #35b19b;  /* Randfarbee oben, rechts, unten, links */
      } 
          
    </style>    
  
  </head>  
  
<body>

  <h1>Robot-Auto-Steuerung</h1>
    
  <!-- Geschwindigkeitsanzeige -->  
  <div id="progressbar">
    <div></div>
  </div>    
  <hr>  
    
  <!-- Geschwindigkeitsteuerung -->
  <div id ="speedContainer">  
    <div class="alleSpeed">
      <a class="speedFlaeche langsam" href="/motor?fahr=langsamer">< langsamer</a>
      <a class="speedFlaeche schnell" href="/motor?fahr=schneller">schneller ></a>
    </div>  
  </div>  

  <!-- Richtungssteuerung -->   
  <div class="panelContainer">
    <div class="panelContainerZeile">       
      <div class="panelParent">     
        <div class="panelChild1">
          <a class="panelChild1-href" href="/motor?fahr=vor">V</a>
        </div>
        <div id="panelChild2V"></div>     
      </div>      
    </div>
        
    <div class="panelContainerZeile">               
      <div class="panelParent">     
        <div class="panelChild1">
          <a class="panelChild1-href" href="/motor?fahr=links">L</a>
        </div>
        <div id="panelChild2L"></div>     
      </div>
      <div>   </div>
      <div class="panelParent">     
        <div class="panelChild1">
          <a class="panelChild1-href" href="/motor?fahr=stop">Stop</a>
        </div>
        <div id="panelChild2S"></div>     
        </div>
      <div>   </div>
      <div class="panelParent">     
        <div class="panelChild1">
          <a class="panelChild1-href" href="/motor?fahr=rechts">R</a>
        </div>
        <div id="panelChild2R"></div>     
      </div>          
    </div>                              
    <div class="panelContainerZeile">               
      <div class="panelParent">     
        <div class="panelChild1">
          <a class="panelChild1-href" href="/motor?fahr=rueck">R</a>
        </div>
      <div id="panelChild2H"></div>     
      </div>            
    </div>                
  
  </div>    

</body>

</html>)motorHTML2";
