function graphmask(infile,outfile,outlines)
%function for plotting MANGLE polygon files using the Matlab mapping toolbox.  
%arguments: 
%infile=polygon file to be plotted, in 'list' format, i.e.created with poly2poly -ol30 mypolys.pol mypolys.list
%outfile=name of eps file to output, or use 'none' if you want to, e.g.,put the resulting figure into a subplot.
%outlines=whether to draw black outlines around polygons. default is no outlines.  Use outlines='on' to draw outlines.

%read in files
weightfile=[infile '.weight'];
xymat=load(infile);
wmat=load(weightfile);
fprintf(1,'done reading files\n');
ra=xymat(1:end,1);
dec=xymat(1:end,2);
weight=wmat(1:end,2);
%set up map axes
axm=axesm('MapProjection', 'hammer','frame','on','FFaceColor','black','origin',180);
%plot polygons in list as patches
%if (strcmp(outlines,'on'))
    h=patchesm(dec,ra,'g','Edgecolor','black','Linewidth',0.3);
%else
%    h=patchesm(dec,ra,'g','Edgecolor','none');
%end
%create cell array of colors with each element as grayscale weight
color=[weight weight weight];
cellcolor=num2cell(color,2);
%apply weight colors to patch objects
set(h,{'FaceColor'},cellcolor);

%tweak map display and add labels
tightmap;
xlims=get(gca,'XLim');
ylims=get(gca,'YLim');
axis([1.01*xlims 1.01*ylims])
set(gca,'XDir', 'reverse','XColor',[1 1 1],'YColor',[1 1 1])
xlabel('-90^{\circ}','Color','k')
ylabel('360^{\circ}','Rotation',0.0,'Color','k','VerticalAlignment','Middle','HorizontalAlignment','Right')
ax1=gca;
ax2=copyobj(ax1,gcf);
set(ax2,'XAxisLocation','top', 'YAxisLocation','right','Color','none','XColor',[1 1 1],'YColor',[1 1 1]);
axes(ax2);
xlabel('+90^{\circ}','Color','k')
ylabel('0^{\circ}','Rotation',0.0,'Color','k','VerticalAlignment','Middle','HorizontalAlignment','Left')

fprintf(1,'Done making mask image\n');
%export image as eps file
if(~strcmp(outfile,'none'))
    set(gcf,'PaperPosition', [-1.75 .75 14 7])
    print('-depsc','-r600',outfile);
    fprintf(1,'Done writing mask image to %s\n',outfile);
    exit
end 
