# BinanceDataStructForOrderBook

Here's the link to the diagrams: https://miro.com/app/board/uXjVHdNlbTQ=/?share_link_id=234326839882

It will contain the workflow of pretty much everything if something isn't clear. It also has additional notes to help you understand the ideas that I have :D

I used a dev container to run this. It's very easy to replicate and I hope you do too!
Just download the Dev containers and WSL(if you are on windows) extensions and clone the repo!
​
image.png

Once the repo is downloaded run the setup.sh and it will prepare everything automatically(I tested it on fresh instances and it should work well enough).
If you want to test the offline version - build.sh
If you want to test the version that I used for testing - binance_api.sh

The base version has a cap level of 20 price levels(I saw that Binance use that). If you add --extended it will run a version(in both scripts),
which doesn't have a cap level.
