# TCP-Proxy  
**Redes I - Trabalho GB**  
Este trabalho visa implementar um proxy entre uma conexão TCP cliente-servidor e analisar o seu impacto em métricas como RTT, CWND, SSTHRESH, entre outras.

# Como Navegar pelo Repositório

Os gráficos dos cenários simulados estão nas pastas **Proxy** e **Client**.

- **Cenário 1:** RTT baixo e 0% de perda  
- **Cenário 2:** 50 ms de atraso e 1% de perda  
- **Cenário 3:** 100 ms de atraso e 2% de perda  
- **Cenário 4:** Limitação de banda (5 Mbps)

## Pasta Proxy
- Contém gráficos da conexão **Proxy → Servidor**  
- Cada pasta *Cenário* contém os gráficos correspondentes aos cenários descritos acima.

## Pasta Client
- Contém gráficos da conexão direta **Cliente → Servidor**  
- Cada pasta *Cenário* contém os gráficos correspondentes aos cenários descritos acima.

## Arquivos .c
Arquivos utilizados para execução do trabalho:

- `clienttcp.c`
- `servidortcp.c`
- `proxytcp.c`

## Arquivo .py
Arquivo que plota os gráficos a partir das simulações realizadas.
- `generatecharts.py`

#Como Reproduzir o Experimento:
- Foram usadas 3 Virtual Machines.
- Cada uma roda um arquivo - proxytcp.c, clientetcp.c, servidortcp.c - simulando uma rede real.
- Instruções sobre cada arquivo abaixo.
- Para geração de gráficos o arquivo .py deve ser executado no mesmo local que o clienttcp.c e proxytcp.c

## proxytcp.c
Aguarda a conexão do cliente, recebe os dados, encaminha para o servidor e todas as métricas relevantes são coletadas durante esse processo. Gera o .csv *metrics* com as medições da simulação.
**Como utilizar:**
- compilar o código: `gcc proxytcp.c -o <nome do executável>`
- executar o programa passado os parametros: `./<nome do executável> <proxy_port> <server_ip> <server_port> <msg_size>`
- <proxy_port>: número da porta do proxy
- <server_ip>: ip do servidor
- <server_port>: porta do servidor
- <msg_size>: tamanho do buffer do proxy

## clienttcp.c 
Envia mensagens para um servidor (ou proxy). Gera o .csv *metrics* com as medições da simulação.
**Como utilizar:**
- compilar o código: `gcc clienttcp.c -o <nome do executável>`
- executar o programa passado os parametros: `./<nome do executável> <remote_host> <remote_port> <msg_size> <num_msgs>`
- <remote_host>: ip do proxy (ou servidor)
- <remote_port>: porta do proxy
- <msg_size>: tamanho do buffer do cliente
- <num_msgs>: número de mensagens que serão enviadas ao proxy

## servidortcp.c
Recebe mensagens enviadas.
**Como utilizar:**
- compilar o código: `gcc servidortcp.c -o <nome do executável>`
- executar o programa passado os parametros: `./<nome do executável> <local_port> <msg_size>`
- <local_port>: número da porta do proxy
- <msg_size>: tamanho do buffer do servidor

  ## generatecharts.py
  Script para plotar os gráficos das medições.
  **Como utilizar:**
  - executar o programa no mesmo local do arquivo *metrics* gerado pelo clienttcp.c e proxytcp.c: `python3 generatecharts.py`
