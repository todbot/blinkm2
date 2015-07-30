


Example JSON version of BlinkM2 light script
{
    "name" : "foo",
    "pattern":
    [
        { "cmd": "fadeRGB", "rgb": "#FFFFFF", "time": 1000, "ledn": 2 },
        { "cmd": "fadeRGB", "rgb": "#FF0000", "time": 1000, "ledn": 3 },
        { "cmd": "fadeRGB", "rgb": "#00FF00", "time": 1000, "ledn": 0 },
        { "cmd": "fadeRGB", "rgb": "#0000FF", "time": 1000, "ledn": 3 },
        { "cmd": "fadeRGB", "rgb": "#000000", "time": 1000, "ledn": 128 },
    ]
}

{
    { "cmd", "play", "id", 8, "start", 3, "len", 2, "reps", 2 },
    { "cmd", "jump", "pos", -3 },
    { "cmd", "roate", "amount", 1 },
    { "cmd", "fadeToHSV", "hsv", "#00FF80", ... },
    { "cmd", "randomRGB", "rgb", "#FF00FF", },
    
    
}
{
    "pattern0" :

    [ { "cmd": "fadeRGB", "rgb": "#00FF00", "time": 1000, "ledn": 0 },
      { "cmd": "fadeRGB", "rgb": "#0000FF", "time": 1000, "ledn": 3 },
      { "cmd": "fadeRGB", "rgb": "#000000", "time": 1000, "ledn": 129 } ],
    
    "pattern1" :
    
    [ { "cmd": "fadeRGB", "rgb": "#FF0000", "time":   0 },
      { "cmd": "fadeRGB", "rgb": "#0000FF", "time": 400 } ],

    "pattern8" :
    
    [ { "cmd": "fadeRGB", "rgb": "#003300", "time": 100 },
      { "cmd": "fadeRGB", "rgb": "#00ff00", "time": 100 },
      { "cmd": "fadeRGB", "rgb": "#666600", "time": 100 }, 
      { "cmd": "fadeRGB", "rgb": "#000000", "time":  50 },
      { "cmd": "fadeRGB", "rgb": "#FFFFFF", "time": 400 }, 
      { "cmd": "fadeRGB", "rgb": "#000000", "time":  50 } ],

    "patternplay" : 

    [ { "cmd": "fadeRGB", "rgb": "#000000", "time": 100 },
      { "cmd": "play", "id": 8, "start": 2, "len": 2, "reps": 4 },
      { "cmd": "play", "id": 8, "start": 4, "len": 2, "reps": 3 } ],

    "pattercylon" :

    [ { "cmd": "fadeRGB", "rgb": "#FF0000", "time": 500, "ledn": 1 },
      { "cmd": "rotate",  "amount": 1 },
      { "cmd": "jump",    "pos": -1 } ],

    "patternfire" :

    [ { "cmd": "randomRGB",  "rgb": "#FFCC00", "range": 40, "time":100, "ledn": 1 },
      { "cmd": "rotate",     "amount": 1 },
      { "cmd": "randomTime", "min": 50, "range":100 },
      { "cmd": "jump",       "pos": -3 } ],

    "patterninput1":
    
    [ { "cmd": "input",   "id": 1, "jumpOnLow": 3 },
      { "cmd": "fadeRGB", "rgb": "#ff00ff", "time": 100, "ledn": 0 },
      { "cmd": "jump",    "pos": -2 },
      { "cmd": "fadeRGB", "rgb": "#00ff00", "time": 100, "ledn": 0 } ],
           

}
   

```
