M = (0.1586 - 0.0051) * 2 + 0.1463 + 0.0205 * 2 + 0.2491;
m = 0.1885 + 0.0458 + 0.0311;
l = 0.04;
0.2491/(m+M)

I = 0.1586 * 0.01^2 + 0.2491 * 0.02^2 + 0.1885 * 0.047^2 + 0.0311 * 0.03^2 + 0.0458 * 0.03^2;





% M = 0.9;
% m = 0.3;
b = 0.1;
% I = 0.01;
g = 9.81;
% l = 0.18;
q = (M+m)*(I+m*l^2)-(m*l)^2;
s = tf('s');

P_cart = (((I+m*l^2)/q)*s^2 - (m*g*l/q))/(s^4 + (b*(I + m*l^2))*s^3/q - ((M + m)*m*g*l)*s^2/q - b*m*g*l*s/q);

P_pend = (m*l*s/q)/(s^3 + (b*(I + m*l^2))*s^2/q - ((M + m)*m*g*l)*s/q - b*m*g*l/q);

sys_tf_cart = [P_cart];
sys_tf_pend = [P_pend];

% inputs = {'u'};
% outputs = {'x'; 'phi'};
% 
% set(sys_tf,'InputName',inputs)
% set(sys_tf,'OutputName',outputs)

sys_tf_cart
sys_tf_pend

C = pidtune(sys_tf_cart,'PID')
D = pidtune(sys_tf_pend,'PID')

% C = pidtune(sys_tf,'PID');