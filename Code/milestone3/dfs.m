%DFS
%represent nodes of maze using raster coordinates (idk if this is actually 
%what these are called but just look at the picture below)
% numbers refer to the coordinates of the maze
%   _ _ _        _ _ _ 
%  |1|2|3|      |1 2|3|
%   - - -        -   -
%  |4|5|6|      |4|5|6|
%   - - -        -   -
%  |7|8|9|      |7 8 9|
%   _ _ _        _ _ _

height = 4;
width  = 5;

adj = zeros(width*height); %
%make sure lower index < upper index

%3x3 adjacency -- be sure to set width 3, height 3
% % adj(1,2) = 1;
% % adj(2,5) = 1;
% % adj(5,8) = 1;
% % adj(7,8) = 1;
% % adj(8,9) = 1;


%4x5 adjacency -- height = 4, width = 5
adj(1,2) = 1;
adj(2,3) = 1;
adj(3,8) = 1;
adj(7,8) = 1;
adj(6,7) = 1;
adj(6,11)= 1;
adj(11,16)=1;
adj(16,17)=1;
adj(12,17)=1;
adj(12,13)=1;
adj(8,13)=1;
adj(13,18)=1;
adj(18,19)=1;
adj(14,19)=1;
adj(14,15)=1;
adj(15,20)=1;
adj(10,15)=1;
adj(4,9)=1;
adj(10,5)=1;
adj(8,9) =1;
adj(11,12)=1;
adj(7,12) = 1;

adj = adj + triu(adj,1).';


xlim([0 width])
ylim([0 height])

global visited;
visited = zeros(height*width,1); %stores 1 if a space has been visited, 0 o/w


global YourImage
global ImageAlpha
[YourImage, ~, ImageAlpha] = imread('cabbage.png');

% initialize gif
figure
frame = getframe(gcf);
img =  frame2im(frame);
[img,cmap] = rgb2ind(img,256);
imwrite(img,cmap,'animation.gif','gif','LoopCount',Inf,'DelayTime',.2);


search(1,width,height,adj)

function search(v,width,height,adj)
     clf
     hold on
     drawBaseGraph(height,width,adj)
     axis off
     del = .0;
     
     %label v as discovered
     global visited
     visited(v) = 1;
     
     [row,col] = rToC(v,width,height); %get the row and column of the current node
     
     frontier = [];
     
     %text(col - .7,row - .5, 'AAAAA', 'FontName','Comic Sans MS','Color','r','FontSize',20);
     global YourImage
     global ImageAlpha
     image([(col-.1) (col-.9)],[(row-.1) (row-.9)], YourImage, 'AlphaData', ImageAlpha)
     writeToGif
     pause(del)
     
     % Capture the plot as an image 
     
     %check if you can move left
     if (col>1 && adj(v,v-1) == 1)
         frontier(length(frontier) + 1) = v-1;
     end
     
     %check if you can move right
     if (col < width && adj(v,v+1) == 1)
         frontier(length(frontier) + 1) = v+1;
     end
     
     %check if you can move up
     if (row < height && adj(v,v-width) == 1)
         frontier(length(frontier) + 1) = v-width;
     end
     
     %check if you can move down
     if (row > 1 && adj(v,v+width) == 1)
         frontier(length(frontier) + 1) = v+width;
     end
     %for all edges from v to w in G.adjacentEdges(v) do
     if (length(frontier > 0))
         for k = 1:length(frontier)
             if (visited(frontier(k)) == 0)
                 search(frontier(k),width,height,adj)
                     clf
                     hold on
                     drawBaseGraph(height,width,adj)
                     axis off
                     %text(col - .7,row - .5, 'AAAAA', 'FontName','Comic Sans MS','Color','r','FontSize',20)
                     image([(col-.9) (col-.1)],[(row-.1) (row-.9)], YourImage, 'AlphaData', ImageAlpha)
                     writeToGif
                     pause(del)
             end
         end
     end
     return
end

function [row,col] = rToC(r, width, height)
    row = 1 + ((r-1) - rem(r-1,width))/width;    
    row = abs(row - height) + 1;
    col = mod(r-1,width) + 1;
end

function drawBaseGraph(height,width,adj)
    for k = 1:height*width
        row(k) = 1 + ((k-1) - rem(k-1,width))/width;
        row(k) = abs(row(k) - height) + 1;
        col(k) = mod(k-1,width) + 1;
        txt = sprintf('%i',k);
        text(col(k) - .5,row(k) - .5,txt,'FontWeight','bold')
    end
    for k = 1:height*width
        %draw vertical lines
        if (k >1 && adj(k,k-1) ~=1)             %if wall on left exists
            plot([col(k)-1 col(k)-1],[row(k)-1 row(k)],'g') %left line
        elseif (k == 1)
            plot([col(k)-1 col(k)-1],[row(k)-1 row(k)],'g') %left line
        end

        if (k <height*width && adj(k,k+1) ~=1)  %if wall on left exists
            plot([col(k) col(k)],[row(k)-1 row(k)],'g')     %right line
        elseif (k == height*width)
            plot([col(k) col(k)],[row(k)-1 row(k)],'g')     %right line
        end
    end
    for k = 1:((height)*width)
        %draw horizontal lines
        %plot([col(k)-1 col(k)],[row(k)-1 row(k)-1],'r') %bottom line

        if (k-1 < width)
            plot([col(k)-1 col(k)],[row(k) row(k)],'g')     %top line of top row
        elseif (k >= (width -1)*height)
            plot([col(k)-1 col(k)],[row(k)-1 row(k)-1],'g') %bottom line of bottom row
        else
            if (adj(k,k-width) ~=1)
                plot([col(k)-1 col(k)],[row(k) row(k)],'g') %top line
            end
            if (adj(k,k+width) ~=1)
                plot([col(k)-1 col(k)],[row(k)-1 row(k)-1],'g') %bottom line
            end
        end
    end
    title('what a nice robot','FontSize',20,'FontWeight','bold','Color','g','FontName','Papyrus')
end


function writeToGif
    % draw stuff
        frame = getframe(gcf);
        img =  frame2im(frame);
        [img,cmap] = rgb2ind(img,256);
        imwrite(img,cmap,'animation.gif','gif','WriteMode','append','DelayTime',.2);
end