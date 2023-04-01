data = dlmread('log.dat', '\t');
figure;
subplot(3, 1, 1);
s1 = stem(data(end - 100:end, 1));
title("ax-t");
subplot(3, 1, 2);
s2 = stem(data(end - 100:end, 2));
title("ay-t");
subplot(3, 1, 3);
s3 = stem(data(end - 100:end, 3));
title("az-t");
while true
	data = dlmread('log.dat', '\t',0, 0);
	set(s1, 'ydata', data(end - 100:end, 1));
	set(s2, 'ydata', data(end - 100:end, 2));
	set(s3, 'ydata', data(end - 100:end, 3));
	pause(0.1);
endwhile
