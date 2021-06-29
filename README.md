![img.jpg](img.jpg)

<br>
developer : Han Chan Hee<br>
student id : 20182204<br>
date : 2021-06-22 ~ 2021-06-24<br>
<br>

> Shell command must be lowercase!<br>
> 2 projects are in the 20182204/ folder<br> 
> project1 : event - based stock server<br>
> project2 : thread based stock server<br>
<br>
<br>

**Compile**<br>
    enter (make) to compile<br>
<br><br>

**Run**<br>
    server run : ./stockserver `<port number>`<br>
    client run : ./stockclient `<server ip>` `<port number>`<br>
    multiclient run : ./multiclient `<server ip>` `<port number>` `<user number>` <br>
<br>

** example **
![img1.png](img1.png)
<br><br>

**delete**<br>
    enter (make clean)<br>
    it removes all execute files<br>
<br><br>

**command**<br>
    - show : show all the items in stock server<br>
    - buy (stock id) (stock num) : buy stock if there is not enough stock then msg is resented. stock price is not changed<br>
    - sell (stock id) (stock num) : sell stock<br>
    - exit : exit the program<br>