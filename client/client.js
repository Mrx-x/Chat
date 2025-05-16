class WebSocketService {
  constructor(url) {
    this.url = url;
    this.socket = null;
    this.onMessageCallback = () => {};
    this.onOpenCallback = () => {};
    this.onCloseCallback = () => {};
  }

  connect() {
    this.socket = new WebSocket(this.url);
    this.socket.onopen    = () => this.onOpenCallback();
    this.socket.onmessage = e => this.onMessageCallback(JSON.parse(e.data));
    this.socket.onclose   = e => this.onCloseCallback(e);
  }

  send(payload) {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      this.socket.send(JSON.stringify(payload));
    }
  }

  onMessage(fn) { this.onMessageCallback = fn; }
  onOpen(fn)    { this.onOpenCallback = fn; }
  onClose(fn)   { this.onCloseCallback = fn; }
}

class UIManager {
  constructor() {
    this.loginDiv    = document.getElementById('login');
    this.chatDiv     = document.getElementById('chat');
    this.loginForm   = document.getElementById('loginForm');
    this.urlInput    = document.getElementById('serverUrl');
    this.nameInput   = document.getElementById('username');
    this.usersList   = document.getElementById('users');
    this.topicsList  = document.getElementById('topics');
    this.messagesDiv = document.getElementById('messages');
    this.inputField  = document.getElementById('messageInput');
    this.sendButton  = document.getElementById('sendBtn');
    this.unread      = { users: {}, topics: {} };
    this.currentChat = null;

    this.inputField.addEventListener('keydown', (e) => {
      if (e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault();
        this.sendButton.click();
      }
    })
  }

  bindLogin(handler) {
    this.loginForm.onsubmit = e => {
      e.preventDefault();
      handler(this.urlInput.value.trim(), this.nameInput.value.trim());
    };
  }

  showChat() {
    this.loginDiv.style.display = 'none';
    this.chatDiv.style.display  = 'flex';
  }

  renderUsers(users, onSelect) {
    this.usersList.innerHTML = '';
    users.forEach(u => {
      const li = document.createElement('li');
      li.textContent = u.name;
      li.dataset.id = u.id;
      const cnt = this.unread.users[u.id] || 0;
      if (cnt > 0) {
        const badge = document.createElement('span');
        badge.className = 'badge';
        badge.textContent = cnt;
        li.appendChild(badge);
      }
      li.onclick = () => onSelect('user', u.id);
      if (this.currentChat?.type === 'user' && this.currentChat.id === u.id) {
        li.classList.add('active');
      }
      this.usersList.appendChild(li);
    });
  }

  renderTopics(topics, onSelect) {
    this.topicsList.innerHTML = '';
    topics.forEach(t => {
      const li = document.createElement('li');
      li.textContent = t;
      li.dataset.id = t;
      const cnt = this.unread.topics[t] || 0;
      if (cnt > 0) {
        const badge = document.createElement('span');
        badge.className = 'badge';
        badge.textContent = cnt;
        li.appendChild(badge);
      }
      li.onclick = () => onSelect('topic', t);
      if (this.currentChat?.type === 'topic' && this.currentChat.id === t) {
        li.classList.add('active');
      }
      this.topicsList.appendChild(li);
    });
  }

  selectChat(type, id) {
    this.currentChat = { type, id };
    [...this.usersList.children].forEach(li =>
      li.classList.toggle('active', type==='user'  && li.dataset.id===id)
    );
    [...this.topicsList.children].forEach(li =>
      li.classList.toggle('active', type==='topic' && li.dataset.id===id)
    );
    if (type==='user')  delete this.unread.users[id];
    if (type==='topic') delete this.unread.topics[id];
  }

  clearMessages() {
    this.messagesDiv.innerHTML = '';
  }
  
  clearUserUnread(type, id, users, topics, selectHandler) {
    if (type === 'user') {
      delete this.unread.users[id];
      this.renderUsers(users, selectHandler);
    } else {
      delete this.unread.topics[id];
      this.renderTopics(topics, selectHandler);
    }
  }

  appendMessage(from, text) {
    const d = document.createElement('div');
    d.className = 'message' + (from === 'Me' ? ' own' : '');
    console.log(text);
    d.textContent = `${from}: ${text}`;
    this.messagesDiv.appendChild(d);
    this.messagesDiv.scrollTop = this.messagesDiv.scrollHeight;
  }

  bindSend(handler) {
    this.sendButton.onclick = () => {
      const txt = this.inputField.value.trim();
      if (!txt || !this.currentChat) return;
      handler(this.currentChat, txt);
      this.inputField.value = '';
    };
  }

  markUnread(type, id) {
    const dict = type==='user' ? this.unread.users : this.unread.topics;
    dict[id] = (dict[id]||0) + 1;
  }
}

class ChatController {
  constructor() {
    this.ui         = new UIManager();
    this.ws         = null;
    this.users      = [];
    this.topics     = new Set();
    this.currentId  = null;
    this.history    = {};
  }

  init() {
    this.ui.bindLogin((url,name)=>this.start(url,name));
  }

  start(url, name) {
    this.ws = new WebSocketService(url);

    this.ws.onOpen(() => {
      this.ui.showChat();
      this.ui.bindSend((chat,text)=>this.send(chat,text));
      this.ws.send({ type:'register', name });
    });

    this.ws.onMessage(msg => this.routeMessage(msg));
    this.ws.onClose(() => this.ui.appendMessage('System','Disconnected'));

    this.ws.connect();
  }

  routeMessage(msg) {
    switch (msg.type) {
      case 'welcome': {
        this.currentId = msg.id;
        this.ws.send({ type:'list' });
        break;
      }

      case 'registered': {
        this.ui.appendMessage('System', `Registered as ${msg.name}`);
        break;
      }
      case 'user_list': {
        this.users = msg.users.filter(u=>u.id!==this.currentId);
        this.ui.clearMessages();
        this.ui.currentChat = null;
        this.ui.renderUsers(this.users, (t,id)=>this.selectChat(t,id));
        break;
      }

      case 'private': {
        const key = `user:${msg.from}`;
        this._saveMessage(key, msg.from, msg.text);
        if (this.ui.currentChat?.type==='user' && this.ui.currentChat.id===msg.from) {
          this.ui.appendMessage(msg.from, msg.text);
        } else {
          this.ui.markUnread('user', msg.from);
          this.ui.renderUsers(this.users, (t,id)=>this.selectChat(t,id));
        }
        break;
      }

      case 'joined': {
        this.topics.add(msg.topic);
        this.ui.renderTopics(Array.from(this.topics), (t,id)=>this.selectChat(t,id));
        break;
      }

      case 'topic': {
        const key = `topic:${msg.topic}`;
        this._saveMessage(key, msg.from, msg.text);
        if (this.ui.currentChat?.type==='topic' && this.ui.currentChat.id===msg.topic) {
          this.ui.appendMessage(`${msg.from}`, msg.text);
        } else {
          this.ui.markUnread('topic', msg.topic);
          this.ui.renderTopics(Array.from(this.topics), (t,id)=>this.selectChat(t,id));
        }
        break;
      }

      case 'disconnect': {
        const disconnectId = msg.id;
        const key = `user:${disconnectId}`;

        delete this.history[key];

        this.users = this.users.filter(u => u.id !== disconnectId);
        this.ui.renderUsers(this.users, (t, id) => this.selectChat(t, id));

        if (this.ui.currentChat?.type === 'user' && this.ui.currentChat.id === disconnectId) {
          this.ui.clearMessages();
          this.ui.currentChat = null;
        }
        break;
      }

      default:
        console.warn('Unknown message', msg);
    }
  }

  selectChat(type, id) {
    this.ui.selectChat(type, id);
    this.ui.clearMessages();

    const key = `${type}:${id}`;
    (this.history[key] || []).forEach(m => {
      this.ui.appendMessage(m.from, m.text);
    });

    this.ui.clearUserUnread(
      type,
      id,
      this.users,
      Array.from(this.topics),
      (t, chatId) => this.selectChat(t, chatId)
    );
  }

  send(chat, text) {
    const key = `${chat.type}:${chat.id}`;
    this._saveMessage(key, 'Me', text);

    if (chat.type==='user') {
      this.ws.send({ type:'pm', to: chat.id, text });
      if (this.ui.currentChat?.type==='user' && this.ui.currentChat.id===chat.id) {
        this.ui.appendMessage('Me', text);
      }
    } else {
      this.ws.send({ type:'topic', topic: chat.id, text });
      if (this.ui.currentChat?.type==='topic' && this.ui.currentChat.id===chat.id) {
        this.ui.appendMessage('Me', text);
      }
    }
  }

  _saveMessage(key, from, text) {
    this.history[key] = this.history[key]||[];
    this.history[key].push({ from, text });
  }
}

document.addEventListener('DOMContentLoaded', () => {
  new ChatController().init();
});
